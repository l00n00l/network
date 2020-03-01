#include "id_generator.h"
#include <atomic>
#include <cstdint>
#include <list>
#include <set>

static id_generator g_session_id_generator;

struct id_generator::id_generator_impl {
  uint64 cur_id;
  std::set<uint64> recycled_ids;
  std::atomic_flag writing_flag;
  std::list<std::pair<uint64, uint64>> recycled_range;
  id_generator_impl() : cur_id(0) { writing_flag.clear(); }
};

id_generator::id_generator() {
  impl_ptr = std::make_unique<id_generator_impl>();
}

id_generator::~id_generator() {}

uint64 id_generator::gen() {
  atomic_flag_acquire(impl_ptr->writing_flag);

  auto ret = 0;
  // ���ȷ���ɢ��ֵ, ��Ϊ�����ͷ��ڴ�
  if (impl_ptr->recycled_ids.size() > 0) {
    ret = *impl_ptr->recycled_ids.begin();
    impl_ptr->recycled_ids.erase(impl_ptr->recycled_ids.begin());
  }

  // �ӷ�Χֵ����һ��
  else if (impl_ptr->recycled_range.size() > 0) {
    auto iter = impl_ptr->recycled_range.begin();
    ret = iter->first;
    if (++(iter->first) == iter->second) {
      //�����Χֻʣ��һ��ֵ�󣬽�ֵ������ɢ���У�ɾ����Χ����
      impl_ptr->recycled_ids.insert(iter->first);
      impl_ptr->recycled_range.erase(iter);
    }
  }
  // �ѵ�ǰֵ����
  else {
    ret = ++impl_ptr->cur_id;
  }

  atomic_flag_release(impl_ptr->writing_flag);

  return ret;
}

void id_generator::recycle(uint64 value) {
  atomic_flag_acquire(impl_ptr->writing_flag);

  impl_ptr->recycled_ids.insert(value);

  auto itr = impl_ptr->recycled_ids.find(value);

  auto left = value;
  auto right = value;

  // ���ǰһλ
  if (itr != impl_ptr->recycled_ids.begin() && value - *--itr == 1) {
    left = *--itr;
  }

  // ����һλ
  if (itr != impl_ptr->recycled_ids.end() &&
      ++itr != impl_ptr->recycled_ids.end() && *++itr - value == 1) {
    right = *++itr;
  }

  if (left != right) {
    // Ѱ��λ�ã��ϲ���Χ
    auto find_insert_position = false;
    for (auto &i : impl_ptr->recycled_range) {
      // �����ϲ�iter1(1,2) + iter2(3,4) ->iter2(1,4)
      if (i.first - right == 1) {
        i.first = left;
        find_insert_position = true;
        break;
      }
      // �����ϲ�iter1(3,4) + iter2(1,2) ->iter2(1,4)
      else if (left - i.second == 1) {
        i.second = right;
        find_insert_position = true;
        break;
      }
    }

    // û�ҵ��ϲ�λ�ã����뷶Χ
    if (!find_insert_position) {
      impl_ptr->recycled_range.push_back(std::make_pair(left, right));
    }

    // ��ɢ�ж�������ɾ��
    for (auto i = left; i <= right; ++i) {
      impl_ptr->recycled_ids.erase(i);
    }

    // �ϲ���Χ���У������Ƿ��ܹ�����
    // ��������䶯��˵�����Ժϲ������������û�з����䶯�����ٽ��кϲ�
    bool need_concat_flag = false; //�Ƿ����䶯���Ƿ����ٴκϲ�������
    do {
      need_concat_flag = false;
      bool remove_flag = false;
      for (auto iter1 = impl_ptr->recycled_range.begin();
           iter1 != impl_ptr->recycled_range.end();) {
        remove_flag = false;
        for (auto iter2 = impl_ptr->recycled_range.begin();
             iter2 != impl_ptr->recycled_range.end(); ++iter2) {
          // �����ϲ�iter1(1,2) + iter2(3,4) ->iter2(1,4)
          if (iter2->first - iter1->second == 1) {
            iter2->first = iter1->first;
            remove_flag = true;
            break;
          }
          // ���Ҳ�ϲ�iter1(3,4) + iter2(1,2) ->iter2(1,4)
          else if (iter1->first - iter2->second == 1) {
            iter2->second = iter2->second;
            remove_flag = true;
            break;
          }
        }

        // �ϲ����Ƴ�iter1
        if (remove_flag) {
          iter1 = impl_ptr->recycled_range.erase(iter1);
          if (!need_concat_flag) {
            need_concat_flag = true;
          }
        } else {
          ++iter1;
        }
      }
    } while (need_concat_flag);
  }

  atomic_flag_release(impl_ptr->writing_flag);
}
