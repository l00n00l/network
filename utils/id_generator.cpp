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
  // 优先分配散列值, 因为可以释放内存
  if (impl_ptr->recycled_ids.size() > 0) {
    ret = *impl_ptr->recycled_ids.begin();
    impl_ptr->recycled_ids.erase(impl_ptr->recycled_ids.begin());
  }

  // 从范围值中拿一个
  else if (impl_ptr->recycled_range.size() > 0) {
    auto iter = impl_ptr->recycled_range.begin();
    ret = iter->first;
    if (++(iter->first) == iter->second) {
      //如果范围只剩下一个值后，将值放入离散队列，删除范围缓存
      impl_ptr->recycled_ids.insert(iter->first);
      impl_ptr->recycled_range.erase(iter);
    }
  }
  // 把当前值自增
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

  // 检测前一位
  if (itr != impl_ptr->recycled_ids.begin() && value - *--itr == 1) {
    left = *--itr;
  }

  // 检测后一位
  if (itr != impl_ptr->recycled_ids.end() &&
      ++itr != impl_ptr->recycled_ids.end() && *++itr - value == 1) {
    right = *++itr;
  }

  if (left != right) {
    // 寻找位置，合并范围
    auto find_insert_position = false;
    for (auto &i : impl_ptr->recycled_range) {
      // 向左侧合并iter1(1,2) + iter2(3,4) ->iter2(1,4)
      if (i.first - right == 1) {
        i.first = left;
        find_insert_position = true;
        break;
      }
      // 向左侧合并iter1(3,4) + iter2(1,2) ->iter2(1,4)
      else if (left - i.second == 1) {
        i.second = right;
        find_insert_position = true;
        break;
      }
    }

    // 没找到合并位置，插入范围
    if (!find_insert_position) {
      impl_ptr->recycled_range.push_back(std::make_pair(left, right));
    }

    // 从散列队列里面删除
    for (auto i = left; i <= right; ++i) {
      impl_ptr->recycled_ids.erase(i);
    }

    // 合并范围队列，看看是否能够接上
    // 如果发生变动就说明可以合并，如果整下来没有发生变动，则不再进行合并
    bool need_concat_flag = false; //是否发生变动，是否有再次合并的需求
    do {
      need_concat_flag = false;
      bool remove_flag = false;
      for (auto iter1 = impl_ptr->recycled_range.begin();
           iter1 != impl_ptr->recycled_range.end();) {
        remove_flag = false;
        for (auto iter2 = impl_ptr->recycled_range.begin();
             iter2 != impl_ptr->recycled_range.end(); ++iter2) {
          // 向左侧合并iter1(1,2) + iter2(3,4) ->iter2(1,4)
          if (iter2->first - iter1->second == 1) {
            iter2->first = iter1->first;
            remove_flag = true;
            break;
          }
          // 向右侧合并iter1(3,4) + iter2(1,2) ->iter2(1,4)
          else if (iter1->first - iter2->second == 1) {
            iter2->second = iter2->second;
            remove_flag = true;
            break;
          }
        }

        // 合并后移除iter1
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
