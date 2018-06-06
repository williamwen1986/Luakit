#ifndef weak_cache_h
#define weak_cache_h

#include "base/threading/non_thread_safe.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include <memory>
#include <functional>
#include <vector>
#include <deque>
#include <map>

/**
 * @brief 该模板类实现了弱引用对象缓存，用于在保证对象唯一性的前提下，丢弃一些未使用的对象以释放内存。\n
 * 这是一个公共基类，根据工程中实际使用的智能指针类型，请选用下面的某一个子类：\n
 * @c std::WeakCache 是针对 @c std::shared_ptr 的特化；\n
 * @c base::WeakCache 是针对 chromium 中 @c scoped_refptr 的特化。
 *
 * 内部包含一个强引用列表，用于存储最近放入缓存的对象，这些对象即使外部没有持有引用，也能在内存中保持，达到缓存的作用。\n
 * 在若干次获取操作之后，会自动对弱引用缓存进行一次清理，删除其中已经无效的对象。
 *
 * @see std::WeakCache
 * @see base::WeakCache
 * @note 暂时未实现LRU，缓存是按照FIFO的方式实现的
 */
template <class K, class V, class RefPtr, class WeakPtr>
class WeakCacheBase : base::NonThreadSafe {
    // 缓存中保持的强引用最大个数的默认值
    static const size_t DEFAULT_CAPACITY = 32;
    // 缓存自动清理无效弱引用的间隔(单位：缓存访问次数)
    static const size_t AUTO_PURGE_COUNT = 256;
    // 缓存自动清理无效弱引用的时间间隔(单位：毫秒)
    static const int AUTO_PURGE_INTERVAL = 1000;
    
    // 对象弱引用缓存
    std::map<K, WeakPtr> _cache_storage;
    // 对象强引用缓存
    std::deque<RefPtr> _strong_deque;
    // 缓存中保持的强引用的最大个数
    size_t _capacity;
    // 访问不存在的值时，调用这个函数对象来创建
    std::function<RefPtr(const K& key)> _creator;
    // 在从缓存中移除一个值后的某个时刻，调用这个函数对象（注意这个调用并不是实时的）
    std::function<void(const K& key)> _cleaner;
    // 自动清理缓存计数器
    size_t _auto_purge_count;
    // 自动清理缓存计时器
    base::TimeTicks _auto_purge_ticks;
  
public:
    typedef typename std::map<K, WeakPtr>::iterator iterator;
    typedef typename std::map<K, WeakPtr>::const_iterator const_iterator;
  
public:
    /**
     * @brief 构造一个弱引用缓存。
     */
    WeakCacheBase() : _capacity(DEFAULT_CAPACITY), _auto_purge_count(0), _auto_purge_ticks(base::TimeTicks::Now()) { }
    /**
     * @brief 构造一个弱引用缓存，并指定持有强引用的最大个数。
     * @param capacity 持有强引用的最大个数
     */
    WeakCacheBase(size_t capacity) : _capacity(capacity), _auto_purge_count(0), _auto_purge_ticks(base::TimeTicks::Now()) { }
    /**
     * @brief 构造一个弱引用缓存，并指定根据 @p key 来构造值的方法。
     * @param creator 如果请求key对应的值不存在于缓存中，使用此构造器将其创建出来
     */
    WeakCacheBase(std::function<RefPtr(const K& key)> creator) : _capacity(DEFAULT_CAPACITY), _creator(creator), _auto_purge_count(0), _auto_purge_ticks(base::TimeTicks::Now()) { }
    /**
     * @brief 构造一个弱引用缓存，并指定根据 @p key 来构造值的方法。
     * @param creator 如果请求key对应的值不存在于缓存中，使用此构造器将其创建出来
     * @param cleaner 在从缓存中移除一个值后的某个时刻，调用这个函数对象（注意这个调用并不是实时的）
     */
    WeakCacheBase(std::function<RefPtr(const K& key)> creator, std::function<void(const K& key)> cleaner) : _capacity(DEFAULT_CAPACITY), _creator(creator), _cleaner(cleaner), _auto_purge_count(0), _auto_purge_ticks(base::TimeTicks::Now()) { }
    /**
     * @brief 构造一个弱引用缓存，并指定持有强引用的最大个数，以及根据 @p key 来构造值的方法。
     * @param capacity 持有强引用的最大个数
     * @param creator 如果请求key对应的值不存在于缓存中，使用此构造器将其创建出来
     */
    WeakCacheBase(size_t capacity, std::function<RefPtr(const K& key)> creator) : _capacity(capacity), _creator(creator), _auto_purge_count(0), _auto_purge_ticks(base::TimeTicks::Now()) { }
    /**
     * @brief 构造一个弱引用缓存，并指定持有强引用的最大个数，以及根据 @p key 来构造值的方法。
     * @param capacity 持有强引用的最大个数
     * @param creator 如果请求key对应的值不存在于缓存中，使用此构造器将其创建出来
     * @param cleaner 在从缓存中移除一个值后的某个时刻，调用这个函数对象（注意这个调用并不是实时的）
     */
    WeakCacheBase(size_t capacity, std::function<RefPtr(const K& key)> creator, std::function<void(const K& key)> cleaner) : _capacity(capacity), _creator(creator), _cleaner(cleaner), _auto_purge_count(0), _auto_purge_ticks(base::TimeTicks::Now()) { }
    
    virtual ~WeakCacheBase() { }
    
    /**
     * @brief 根据 @p key 从缓存中获取一个值。
     *
     * 此方法查找弱引用缓存中是否存在 @p key 所对应的值。如果存在且不为空，则返回一个指向它的引用计数的智能指针。
     * 如果不存在，则使用构造缓存时传入的 @c creator 来创建这个值，存入缓存并返回一个指向它的引用计数的智能指针。
     *
     * @param key 查找缓存所用的key
     * @return 一个指向该key在缓存中对应值的引用计数智能指针，或是指向空
     * @see insert
     * @note 这个方法不能用于向缓存中插入对象。如果要插入，请调用 @c insert 方法。
     */
    const RefPtr operator[](const K& key) {
        CalledOnValidThread();
        // 当使用缓存一定次数，且距上次清理已有一段时间后，自动清理一次弱引用
        _auto_purge_count++;
        if (_auto_purge_count >= AUTO_PURGE_COUNT) {
            base::TimeDelta delta = base::TimeTicks::Now() - _auto_purge_ticks;
            if (delta >= base::TimeDelta::FromMilliseconds(AUTO_PURGE_INTERVAL)) {
                purge();
            }
        }
        // 在弱引用缓存中查找key
        auto iter = _cache_storage.find(key);
        
        if (iter == _cache_storage.end() || is_weak_ptr_expired(iter->second)) {
            // 如果缓存中没有找到或者已经被释放，则尝试创建一个
            if (_creator) {
                RefPtr ptr = _creator(key);
                insert(key, ptr);
                return ptr;
            } else {
                return RefPtr();
            }
        }
        return as_ref_ptr(iter->second);
    }
    
    /**
     * @brief 向缓存中插入一个键值对。
     *
     * 此方法手动添加一个键值对到弱引用缓存中。
     *
     * @param key
     * @param value_ptr
     * @warning 为了保证对象的唯一性，使用者应保证缓存中没有这个对象，才能手动创建该对象并加入缓存！
     */
    void insert(const K& key, RefPtr value_ptr) {
        CalledOnValidThread();
#if DEBUG
        // 检查缓存中是否已存在同样的key，并且其引用是有效的
        // 如果中了断言，则对象唯一性将无法得到保证！
        auto iter = _cache_storage.find(key);
        DCHECK(iter == _cache_storage.end() || is_weak_ptr_expired(iter->second));
#endif
        // 放入强引用缓存
        if (_strong_deque.size() == _capacity) {
            _strong_deque.pop_front();
        }
        _strong_deque.push_back(value_ptr);
        // 放入弱引用缓存
        _cache_storage[key] = as_weak_ptr(value_ptr);
    }

    /**
     * @brief 获取缓存中所有有效键的一个集合。
     *
     * 由于缓存中仅保存了值的弱引用，通常的遍历过程中进行某些操作有可能导致缓存内容发生改变。
     * 如果希望遍历缓存中所有的有效键，推荐使用此方法取到集合，再对此集合进行遍历。
     * @see dump_values
     * @see dump_pairs
     * @note 调用此方法会立即清理一次缓存中的无效弱引用。
     */
    const std::vector<K> dump_keys() {
        purge();
        std::vector<RefPtr> result;
        result.reserve(_cache_storage.size());
        for (auto& iter : _cache_storage) {
            result.push_back(iter.first);
        }
        return result;
    }
    
    /**
     * @brief 获取缓存中所有有效值的一个集合。
     *
     * 由于缓存中仅保存了值的弱引用，通常的遍历过程中进行某些操作有可能导致缓存内容发生改变。
     * 如果希望遍历缓存中所有的有效值，推荐使用此方法取到强引用集合，再对此集合进行遍历。
     * @see dump_keys
     * @see dump_pairs
     * @note 调用此方法会立即清理一次缓存中的无效弱引用。
     */
    std::vector<RefPtr> dump_values() {
        purge();
        std::vector<RefPtr> result;
        result.reserve(_cache_storage.size());
        for (auto& iter : _cache_storage) {
            result.push_back(as_ref_ptr(iter.second));
        }
        return result;
    }
    
    /**
     * @brief 获取缓存中所有有效键值对的一个集合。
     *
     * 由于缓存中仅保存了值的弱引用，通常的遍历过程中进行某些操作有可能导致缓存内容发生改变。
     * 如果希望遍历缓存中所有的有效键值对，推荐使用此方法取到强引用集合，再对此集合进行遍历。
     * @see dump_keys
     * @see dump_values
     * @note 调用此方法会立即清理一次缓存中的无效弱引用。
     */
    const std::map<K, RefPtr> dump_pairs() {
        purge();
        std::map<K, RefPtr> result;
        result.reserve(_cache_storage.size());
        for (auto& iter : _cache_storage) {
            result.insert(std::make_pair(iter.first, as_ref_ptr(iter.second)));
        }
        return result;
    }
    
    /**
     * @brief 清理映射表中已经无效的弱引用。
     *
     * 此方法扫描并删除缓存中已经失效的弱引用指针，以提高查表性能。
     * 因为自动清理的原因，通常不需要手动调用此方法。
     */
    void purge() {
        CalledOnValidThread();
        _auto_purge_count = 0;
        _auto_purge_ticks = base::TimeTicks::Now();
        for (auto iter = _cache_storage.begin(); iter != _cache_storage.end(); ) {
            if (is_weak_ptr_expired(iter->second)) {
                if (_cleaner) {
                    _cleaner(iter->first);
                }
                iter = _cache_storage.erase(iter);
            } else {
                ++iter;
            }
        }
    }
protected:
    /**
     * @brief 由子类实现，用于判断一个弱引用指针是否已经失效。
     */
    virtual bool is_weak_ptr_expired(const WeakPtr& weak_ptr) const = 0;
    /**
     * @brief 由子类实现，用于将一个引用计数指针转换为一个弱引用指针。
     */
    virtual WeakPtr as_weak_ptr(const RefPtr& ref_ptr) const = 0;
    /**
     * @brief 由子类实现，用于将一个弱引用指针转换为一个引用计数指针。
     */
    virtual RefPtr as_ref_ptr(const WeakPtr& ref_ptr) const = 0;
};

namespace std {
/**
 * @brief 该模板类实现了弱引用对象缓存，用于在保证对象唯一性的前提下，丢弃一些未使用的对象以释放内存。\n
 * @c std::WeakCache 是 @c WeakCacheBase 针对 @c std::shared_ptr 的特化。\n
 * 查看基类 @c WeakCacheBase 以获取更多信息。
 *
 * @see WeakCacheBase
 * @see base::WeakCache
 */
template <class K, class V>
class WeakCache : public WeakCacheBase<K, V, std::shared_ptr<V>, std::weak_ptr<V>> {
public:
    typedef std::shared_ptr<V> RefPtr;
    typedef std::weak_ptr<V> WeakPtr;
    typedef std::function<RefPtr(const K& key)> CreatorType;
    typedef std::function<void(const K& key)> CleanerType;
    
    WeakCache() : WeakCacheBase<K, V, RefPtr, WeakPtr>() { }
    WeakCache(size_t capacity) : WeakCacheBase<K, V, RefPtr, WeakPtr>(capacity) { }
    WeakCache(CreatorType creator) : WeakCacheBase<K, V, RefPtr, WeakPtr>(creator) { }
    WeakCache(CreatorType creator, std::function<void(const K& key)> cleaner) : WeakCacheBase<K, V, RefPtr, WeakPtr>(creator, cleaner) { }
    WeakCache(size_t capacity, CreatorType creator) : WeakCacheBase<K, V, RefPtr, WeakPtr>(capacity, creator) { }
    WeakCache(size_t capacity, CreatorType creator, CleanerType cleaner) : WeakCacheBase<K, V, RefPtr, WeakPtr>(capacity, creator, cleaner) { }
protected:
    virtual bool is_weak_ptr_expired(const WeakPtr& weak_ptr) const {
        return weak_ptr.expired();
    }
    
    virtual WeakPtr as_weak_ptr(const RefPtr& ref_ptr) const {
        return ref_ptr;
    }
    
    virtual RefPtr as_ref_ptr(const WeakPtr& weak_ptr) const {
        return weak_ptr.lock();
    }
};
}

namespace base {
/**
 * @brief 该模板类实现了弱引用对象缓存，用于在保证对象唯一性的前提下，丢弃一些未使用的对象以释放内存。\n
 * @c base::WeakCache 是 @c WeakCacheBase 针对 chromium @c scoped_refptr 的特化。\n
 * 查看基类 @c WeakCacheBase 以获取更多信息。
 *
 * @see WeakCacheBase
 * @see std::WeakCache
 */
template <class K, class V>
class WeakCache : public WeakCacheBase<K, V, scoped_refptr<V>, base::WeakPtr<V>> {
public:
    typedef scoped_refptr<V> RefPtr;
    typedef base::WeakPtr<V> WeakPtr;
    typedef std::function<RefPtr(const K& key)> CreatorType;
    typedef std::function<void(const K& key)> CleanerType;

    WeakCache() : WeakCacheBase<K, V, RefPtr, base::WeakPtr<V>>() { }
    WeakCache(size_t capacity) : WeakCacheBase<K, V, RefPtr,WeakPtr>(capacity) { }
    WeakCache(CreatorType creator) : WeakCacheBase<K, V, RefPtr, WeakPtr>(creator) { }
    WeakCache(CreatorType creator, CleanerType cleaner) : WeakCacheBase<K, V, RefPtr,WeakPtr>(creator, cleaner) { }
    WeakCache(size_t capacity, CreatorType creator) : WeakCacheBase<K, V, RefPtr, WeakPtr>(capacity, creator) { }
    WeakCache(size_t capacity, CreatorType creator, CleanerType cleaner) : WeakCacheBase<K, V, RefPtr,WeakPtr>(capacity, creator, cleaner) { }
protected:
    virtual bool is_weak_ptr_expired(const WeakPtr& weak_ptr) const {
        return weak_ptr.get() == NULL;
    }
    
    virtual WeakPtr as_weak_ptr(const RefPtr& ref_ptr) const {
        return ref_ptr->AsWeakPtr();
    }
    
    virtual RefPtr as_ref_ptr(const WeakPtr& weak_ptr) const {
        return weak_ptr.get();
    }
};
}

#endif /* weak_cache_h */
