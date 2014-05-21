#include "stdafx.h"

#include "gep/ArrayPtr.h"
#include "gep/container/DynamicArray.h"
#include "gep/container/hashmap.h"
#include "gep/container/queue.h"
#include "gep/memory/allocators.h"

using namespace gep;

namespace {
    struct LifetimeCheck
    {
        static int s_constructionCount;
        static int s_destroyCount;
        static int s_moveConstructCount;
        static int s_copyConstructCount;
        static int s_moveCount;
        static int s_copyCount;

        static void reset()
        {
            s_constructionCount = 0;
            s_destroyCount = 0;
            s_moveConstructCount = 0;
            s_copyConstructCount = 0;
            s_moveCount = 0;
            s_copyCount = 0;
        }

        LifetimeCheck()
        {
            s_constructionCount++;
        }

        LifetimeCheck(const LifetimeCheck& rh)
        {
            s_copyConstructCount++;
        }

        LifetimeCheck(LifetimeCheck&& rh)
        {
            s_moveConstructCount++;
        }

        LifetimeCheck& operator = (const LifetimeCheck& rh)
        {
            s_copyCount++;
            return *this;
        }

        LifetimeCheck& operator = (LifetimeCheck&& rh)
        {
            s_moveCount++;
            return *this;
        }

        ~LifetimeCheck()
        {
            s_destroyCount++;
        }
    };

    int LifetimeCheck::s_constructionCount = 0;
    int LifetimeCheck::s_destroyCount = 0;
    int LifetimeCheck::s_moveCount = 0;
    int LifetimeCheck::s_copyCount = 0;
    int LifetimeCheck::s_copyConstructCount = 0;
    int LifetimeCheck::s_moveConstructCount = 0;

    struct LifetimeCheckKey
    {
        static int s_constructionCount;
        static int s_destroyCount;
        static int s_moveConstructCount;
        static int s_copyConstructCount;
        static int s_moveCount;
        static int s_copyCount;
        int value;

        static void reset()
        {
            s_constructionCount = 0;
            s_destroyCount = 0;
            s_moveConstructCount = 0;
            s_copyConstructCount = 0;
            s_moveCount = 0;
            s_copyCount = 0;
        }

        LifetimeCheckKey(int value) : value(value)
        {
            s_constructionCount++;
        }

        LifetimeCheckKey(const LifetimeCheckKey& rh)
        {
            s_copyConstructCount++;
            this->value = rh.value;
        }

        LifetimeCheckKey(LifetimeCheckKey&& rh)
        {
            s_moveConstructCount++;
            this->value = rh.value;
            rh.value = -1;
        }

        LifetimeCheckKey& operator = (const LifetimeCheckKey& rh)
        {
            s_copyCount++;
            this->value = rh.value;
            return *this;
        }

        LifetimeCheckKey& operator = (LifetimeCheckKey&& rh)
        {
            s_moveCount++;
            this->value = rh.value;
            rh.value = -1;
            return *this;
        }

        ~LifetimeCheckKey()
        {
            s_destroyCount++;
            value = -1;
        }

        bool operator == (const LifetimeCheckKey& rh) const
        {
            return value == rh.value;
        }
    };

    int LifetimeCheckKey::s_constructionCount = 0;
    int LifetimeCheckKey::s_destroyCount = 0;
    int LifetimeCheckKey::s_moveCount = 0;
    int LifetimeCheckKey::s_copyCount = 0;
    int LifetimeCheckKey::s_copyConstructCount = 0;
    int LifetimeCheckKey::s_moveConstructCount = 0;

    struct Collision
    {
        unsigned int m_hash;
        int value;

        Collision(unsigned int h, int value)
        {
            this->value = value;
            m_hash = h;
        }

        inline unsigned hash() const { return m_hash; }

        bool operator == (const Collision& rh) const { return this->value == rh.value; }
    };
}


GEP_UNITTEST_GROUP(Container)
GEP_UNITTEST_TEST(Container, DynamicArray)
{
    // Test default construction and single value append
    {
        DynamicArray<int, SimpleLeakCheckingAllocatorPolicy> a1;
        for(int i=0; i<100; i++)
        {
            a1.append(i);
        }
        GEP_ASSERT(SimpleLeakCheckingAllocator::instance().getAllocCount() > 0, "not using allocator");
        GEP_ASSERT(a1.length() == 100);
        for(int i=0; i<100; i++)
        {
            GEP_ASSERT(a1[i] == i, "element not correct", i, a1[i]);
        }
    }
    SimpleLeakCheckingAllocator::destroyInstance(); //causes a memory leak check

    {
        // test construction from an array
        int data[] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
        ArrayPtr<int> arrayData(data);
        DynamicArray<int, SimpleLeakCheckingAllocatorPolicy> a1(arrayData);
        GEP_ASSERT(a1.length() == arrayData.length(), "length not correctly taken from initial data");
        for(int i=0; i<10; i++)
        {
            GEP_ASSERT(a1[i] == 10 - i, "element not correctly copied", i, a1[i]);
        }

        // test move constructor
        int* dataPtr = a1.toArray().getPtr();
        DynamicArray<int, SimpleLeakCheckingAllocatorPolicy> a2(std::move(a1));
        GEP_ASSERT(a1.length() == 0, "data has not been moved out of a1", a1.length());
        GEP_ASSERT(a2.length() == 10, "data has not been moved into a2", a2.length());
        GEP_ASSERT(a2.toArray().getPtr() == dataPtr, "data address should not change on a move");
        for(int i=0; i<10; i++)
        {
            GEP_ASSERT(a2[i] == 10 - i, "element not correctly moved", i, a1[i]);
        }

        // test copy constructor
        DynamicArray<int, SimpleLeakCheckingAllocatorPolicy> a3(a2);
        GEP_ASSERT(a2.length() == a3.length(), "data not correctly copied");
        GEP_ASSERT(a2.toArray().getPtr() != a3.toArray().getPtr(), "data has not been copied");
        for(int i=0; i<10; i++)
        {
            GEP_ASSERT(a3[i] == 10 - i, "element not correctly copied", i, a3[i]);
        }

        // test copy constructor with a different allocator
        DynamicArray<int> a4(a3);
        GEP_ASSERT(a4.length() == a3.length(), "data not correctly copied");
        GEP_ASSERT(a4.toArray().getPtr() != a3.toArray().getPtr(), "data has not been copied");

        // test move constructor with a different allocator
        DynamicArray<int> a5(std::move(a3));
        GEP_ASSERT(a3.length() == 0, "data has not been moved out of a3", a3.length());
        GEP_ASSERT(a5.length() == 10, "data has not been moved into a5", a5.length());
    }
    SimpleLeakCheckingAllocator::instance().destroyInstance(); //checks for memory leaks

    // test assignment operators
    {
        // test append with an array
        int data[] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
        ArrayPtr<int> arrayData(data);
        DynamicArray<int, SimpleLeakCheckingAllocatorPolicy> a1;
        a1.append(arrayData);
        GEP_ASSERT(a1.length() == 10, "array not correctly appended", a1.length());
        for(int i=0; i<10; i++)
        {
            GEP_ASSERT(a1[i] == 10 - i, "element not correctly appended", i, a1[i]);
        }

        // test move assignment
        DynamicArray<int, SimpleLeakCheckingAllocatorPolicy> a2;
        int* dataPtr = a1.toArray().getPtr();
        a2 = std::move(a1);
        GEP_ASSERT(a1.length() == 0, "data has not been moved out of a1", a1.length());
        GEP_ASSERT(a2.length() == 10, "data has not been moved into a2", a2.length());
        GEP_ASSERT(a2.toArray().getPtr() == dataPtr, "data address should not change on a move");
        for(int i=0; i<10; i++)
        {
            GEP_ASSERT(a2[i] == 10 - i, "element not correctly moved", i, a1[i]);
        }

        // test copy assignment
        a1 = a2;
        GEP_ASSERT(a2.length() == a1.length(), "data not correctly copied");
        GEP_ASSERT(a2.toArray().getPtr() != a1.toArray().getPtr(), "data has not been copied");
        for(int i=0; i<10; i++)
        {
            GEP_ASSERT(a1[i] == 10 - i, "element not correctly copied", i, a1[i]);
        }

        // test move assignment with different allocator
        DynamicArray<int> a3;
        dataPtr = a1.toArray().getPtr();
        a3 = std::move(a1);
        GEP_ASSERT(a1.length() == 0, "data has not been moved out of a1", a1.length());
        GEP_ASSERT(a3.length() == 10, "data has not been moved into a2", a2.length());
        GEP_ASSERT(a3.toArray().getPtr() == dataPtr, "data address should not change on a move");

        // test copy assignment with different allocator
        DynamicArray<int> a4;
        a4 = a2;
        GEP_ASSERT(a2.length() == a4.length(), "data not correctly copied");
        GEP_ASSERT(a2.toArray().getPtr() != a4.toArray().getPtr(), "data has not been copied");
    }
    SimpleLeakCheckingAllocator::instance().destroyInstance();

    //test iterator
    {
        int data[] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
        ArrayPtr<int> arrayData(data);
        DynamicArray<int, SimpleLeakCheckingAllocatorPolicy> a1(arrayData);

        int i=0;
        for(int v : a1)
        {
            GEP_ASSERT(v == 10 - i, "iterator not working correctly");
            i++;
        }
        GEP_ASSERT(i == 10, "iterator did iterate a wrong number of elements", i);

        //const version
        const DynamicArray<int, SimpleLeakCheckingAllocatorPolicy>& a2 = a1;

        for(i=0; i<10; i++)
        {
            GEP_ASSERT(a2[i] == 10 - i, "const [] operator not working", i, a2[i]);
        }

        i=0;
        for(int v : a2)
        {
            GEP_ASSERT(v == 10 - i, "const iterator not working correctly");
            i++;
        }
        GEP_ASSERT(i == 10, "iterator did iterate a wrong number of elements", i);
    }
    SimpleLeakCheckingAllocator::destroyInstance();

    //check self assignment
    {
        int data[] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
        ArrayPtr<int> arrayData(data);
        DynamicArray<int, SimpleLeakCheckingAllocatorPolicy> a1(arrayData);

        //self assignment
        a1 = a1;
        GEP_ASSERT(a1.length() == 10, "self assignment not implemented correctly");
        GEP_ASSERT(SimpleLeakCheckingAllocator::instance().getAllocCount() == 1, "self assignment not implemented correctly");
        GEP_ASSERT(SimpleLeakCheckingAllocator::instance().getFreeCount() == 0, "self assignment not implemented correctly");

        //self move assignment
        a1 = std::move(a1);
        GEP_ASSERT(a1.length() == 10, "self move assignment not implemented correctly");
    }
    SimpleLeakCheckingAllocator::destroyInstance();

    // test remove functions
    {
        int data[] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
        ArrayPtr<int> arrayData(data);
        DynamicArray<int, SimpleLeakCheckingAllocatorPolicy> a1(arrayData);

        a1.removeAtIndexUnordered(a1.length() - 1);
        for(int i=0; i < 9; i++)
            GEP_ASSERT(a1[i] == 10 - i, "removeAtIndexUnordered not working correctly for last element");

        a1.removeAtIndexUnordered(0);
        GEP_ASSERT(a1[0] == 2, "removeAtIndexUnordered not working correctly for first element");
        for(int i=1; i < 8; i++)
            GEP_ASSERT(a1[i] == 10 - i, "removeAtIndexUnordered not working correctly for first element");
    }

    //check correct life time handling
    {
        LifetimeCheck::reset();
        LifetimeCheck a[13];
        ArrayPtr<LifetimeCheck> data(a);
        DynamicArray<LifetimeCheck, SimpleLeakCheckingAllocatorPolicy> a1(data);
        GEP_ASSERT(LifetimeCheck::s_constructionCount == 13, "wrong number of elements constructed", LifetimeCheck::s_constructionCount);
        GEP_ASSERT(LifetimeCheck::s_destroyCount == 0, "wrong number of elements destroyed", LifetimeCheck::s_destroyCount);
        GEP_ASSERT(LifetimeCheck::s_copyConstructCount == 13, "wrong number of elements copy constructed", LifetimeCheck::s_copyConstructCount);
        GEP_ASSERT(LifetimeCheck::s_moveConstructCount == 0, "wrong number of elements move constructed", LifetimeCheck::s_moveConstructCount);
        GEP_ASSERT(LifetimeCheck::s_copyCount == 0, "wrong number of elements copied", LifetimeCheck::s_copyCount);
        GEP_ASSERT(LifetimeCheck::s_moveCount == 0, "wrong number of elements moved", LifetimeCheck::s_moveCount);

        a1.append(data);
        GEP_ASSERT(LifetimeCheck::s_constructionCount == 13, "wrong number of elements constructed", LifetimeCheck::s_constructionCount);
        GEP_ASSERT(LifetimeCheck::s_destroyCount == 13, "wrong number of elements destroyed", LifetimeCheck::s_destroyCount);
        GEP_ASSERT(LifetimeCheck::s_copyConstructCount == 26, "wrong number of elements copy constructed", LifetimeCheck::s_copyConstructCount);
        GEP_ASSERT(LifetimeCheck::s_moveConstructCount == 13, "wrong number of elements move constructed", LifetimeCheck::s_moveConstructCount);
        GEP_ASSERT(LifetimeCheck::s_copyCount == 0, "wrong number of elements copied", LifetimeCheck::s_copyCount);
        GEP_ASSERT(LifetimeCheck::s_moveCount == 0, "wrong number of elements moved", LifetimeCheck::s_moveCount);

        DynamicArray<LifetimeCheck, SimpleLeakCheckingAllocatorPolicy> a2(a1);
        GEP_ASSERT(LifetimeCheck::s_constructionCount == 13, "wrong number of elements constructed", LifetimeCheck::s_constructionCount);
        GEP_ASSERT(LifetimeCheck::s_destroyCount == 13, "wrong number of elements destroyed", LifetimeCheck::s_destroyCount);
        GEP_ASSERT(LifetimeCheck::s_copyConstructCount == 52, "wrong number of elements copy constructed", LifetimeCheck::s_copyConstructCount);
        GEP_ASSERT(LifetimeCheck::s_moveConstructCount == 13, "wrong number of elements move constructed", LifetimeCheck::s_moveConstructCount);
        GEP_ASSERT(LifetimeCheck::s_copyCount == 0, "wrong number of elements copied", LifetimeCheck::s_copyCount);
        GEP_ASSERT(LifetimeCheck::s_moveCount == 0, "wrong number of elements moved", LifetimeCheck::s_moveCount);

        DynamicArray<LifetimeCheck, SimpleLeakCheckingAllocatorPolicy> a3;
        a3 = a2;
        GEP_ASSERT(LifetimeCheck::s_constructionCount == 13, "wrong number of elements constructed", LifetimeCheck::s_constructionCount);
        GEP_ASSERT(LifetimeCheck::s_destroyCount == 13, "wrong number of elements destroyed", LifetimeCheck::s_destroyCount);
        GEP_ASSERT(LifetimeCheck::s_copyConstructCount == 78, "wrong number of elements copy constructed", LifetimeCheck::s_copyConstructCount);
        GEP_ASSERT(LifetimeCheck::s_moveConstructCount == 13, "wrong number of elements move constructed", LifetimeCheck::s_moveConstructCount);
        GEP_ASSERT(LifetimeCheck::s_copyCount == 0, "wrong number of elements copied", LifetimeCheck::s_copyCount);
        GEP_ASSERT(LifetimeCheck::s_moveCount == 0, "wrong number of elements moved", LifetimeCheck::s_moveCount);
    }
}

GEP_UNITTEST_TEST(Container, hashmap)
{
  struct Test
  {
    int check;
    int value;

    Test(int value)
    {
      this->check = 5;
      this->value = value;
    }

    Test()
    {
        check = 5;
        value = 0;
    }

    ~Test()
    {
      GEP_ASSERT(check == 5, "check should not have been modified");
    }
  };

  //test string -> test struct
  {
      struct ConstantString
      {
          const char* m_str;
          ConstantString(const char* str) : m_str(str) {}
          bool operator == (const ConstantString&  rh) const { return strcmp(m_str, rh.m_str) == 0; }
          operator const char*() const { return m_str; }
      };

      Hashmap<ConstantString, Test, StringHashPolicy, SimpleLeakCheckingAllocatorPolicy> map1;

      map1[ConstantString("1")] = Test(1);
      map1[ConstantString("2")] = Test(2);
      map1[ConstantString("3")] = Test(3);
      map1[ConstantString("4")] = Test(4);
      map1[ConstantString("5")] = Test(5);

      GEP_ASSERT(map1.exists(ConstantString("1")));
      GEP_ASSERT(map1.exists(ConstantString("2")));
      GEP_ASSERT(map1.exists(ConstantString("3")));
      GEP_ASSERT(map1.exists(ConstantString("4")));
      GEP_ASSERT(map1.exists(ConstantString("5")));

      GEP_ASSERT(map1[ConstantString("1")].value == 1);
      GEP_ASSERT(map1[ConstantString("2")].value == 2);
      GEP_ASSERT(map1[ConstantString("3")].value == 3);
      GEP_ASSERT(map1[ConstantString("4")].value == 4);
      GEP_ASSERT(map1[ConstantString("5")].value == 5);

      GEP_ASSERT(map1.count() == 5);

      auto result = map1.remove(ConstantString("1"));
      GEP_ASSERT(result == SUCCESS);

      GEP_ASSERT(map1.count() == 4);

      GEP_ASSERT(!map1.exists(ConstantString("1")));
      GEP_ASSERT(map1.exists(ConstantString("2")));
      GEP_ASSERT(map1.exists(ConstantString("3")));
      GEP_ASSERT(map1.exists(ConstantString("4")));
      GEP_ASSERT(map1.exists(ConstantString("5")));

      for(auto& entry : map1)
      {
        auto val = atoi(entry.key);
        GEP_ASSERT(entry.value.value == val);
      }

      //test keyRange
      int sum = 0;
      for(auto key : map1.keys())
      {
        int val = atoi(key);
        sum += val;
      }
      GEP_ASSERT(sum == 14);

      //test valueRange
      sum = 0;
      for(auto value : map1.values()) //compiler crash if () is missing
      {
        sum += value.value;
      }
      GEP_ASSERT(sum == 14);
  }
  GEP_ASSERT( SimpleLeakCheckingAllocator::instance().getAllocCount() > 0, "not using allocator");
  //check for memory leaks
  SimpleLeakCheckingAllocator::destroyInstance();

  {
      Hashmap<Collision, int, HashMethodPolicy, SimpleLeakCheckingAllocatorPolicy> map2;

      map2[Collision(0, 0)] = 0;
      map2[Collision(1, 1)] = 1;
      map2[Collision(0, 2)] = 2;
      map2[Collision(1, 3)] = 3;
      map2[Collision(1, 4)] = 4;
      map2[Collision(0, 5)] = 5;

      GEP_ASSERT(map2[Collision(0, 0)] == 0);
      GEP_ASSERT(map2[Collision(1, 1)] == 1);
      GEP_ASSERT(map2[Collision(0, 2)] == 2);
      GEP_ASSERT(map2[Collision(1, 3)] == 3);
      GEP_ASSERT(map2[Collision(1, 4)] == 4);
      GEP_ASSERT(map2[Collision(0, 5)] == 5);

      GEP_ASSERT(map2.exists(Collision(0, 0)));
      GEP_ASSERT(map2.exists(Collision(1, 1)));
      GEP_ASSERT(map2.exists(Collision(0, 2)));
      GEP_ASSERT(map2.exists(Collision(1, 3)));
      GEP_ASSERT(map2.exists(Collision(1, 4)));
      GEP_ASSERT(map2.exists(Collision(0, 5)));

      GEP_ASSERT(map2.remove(Collision(0,6)) == FAILURE);
      GEP_ASSERT(map2.remove(Collision(1,7)) == FAILURE);

      GEP_ASSERT(!map2.exists(Collision(0, 6)));
      GEP_ASSERT(!map2.exists(Collision(1, 7)));

      int value;
      GEP_ASSERT(map2.tryGet(Collision(0, 5), value) == SUCCESS);
      GEP_ASSERT(value == 5);
      GEP_ASSERT(map2.tryGet(Collision(0, 6), value) == FAILURE);
      GEP_ASSERT(value == 5);

      GEP_ASSERT(map2.remove(Collision(0,0)) == SUCCESS);
      GEP_ASSERT(map2.remove(Collision(1,1)) == SUCCESS);

      GEP_ASSERT(map2[Collision(0, 2)] == 2);
      GEP_ASSERT(map2[Collision(1, 3)] == 3);
      GEP_ASSERT(map2[Collision(1, 4)] == 4);
      GEP_ASSERT(map2[Collision(0, 5)] == 5);

      GEP_ASSERT(map2.exists(Collision(0, 2)));
      GEP_ASSERT(map2.exists(Collision(1, 3)));
      GEP_ASSERT(map2.exists(Collision(1, 4)));
      GEP_ASSERT(map2.exists(Collision(0, 5)));

      map2[Collision(0, 6)] = 6;
      map2[Collision(1, 7)] = 7;

      GEP_ASSERT(map2[Collision(0, 2)] == 2);
      GEP_ASSERT(map2[Collision(1, 3)] == 3);
      GEP_ASSERT(map2[Collision(1, 4)] == 4);
      GEP_ASSERT(map2[Collision(0, 5)] == 5);
      GEP_ASSERT(map2[Collision(0, 6)] == 6);
      GEP_ASSERT(map2[Collision(1, 7)] == 7);

      GEP_ASSERT(map2.exists(Collision(0, 2)));
      GEP_ASSERT(map2.exists(Collision(1, 3)));
      GEP_ASSERT(map2.exists(Collision(1, 4)));
      GEP_ASSERT(map2.exists(Collision(0, 5)));
      GEP_ASSERT(map2.exists(Collision(0, 6)));
      GEP_ASSERT(map2.exists(Collision(1, 7)));

      GEP_ASSERT(map2.remove(Collision(1, 4)) == SUCCESS);
      GEP_ASSERT(map2.remove(Collision(0, 6)) == SUCCESS);

      GEP_ASSERT(map2[Collision(0, 2)] == 2);
      GEP_ASSERT(map2[Collision(1, 3)] == 3);
      GEP_ASSERT(map2[Collision(0, 5)] == 5);
      GEP_ASSERT(map2[Collision(1, 7)] == 7);

      GEP_ASSERT(map2.exists(Collision(0, 2)));
      GEP_ASSERT(map2.exists(Collision(1, 3)));
      GEP_ASSERT(map2.exists(Collision(0, 5)));
      GEP_ASSERT(map2.exists(Collision(1, 7)));

      map2[Collision(0, 2)] = 3;
      map2[Collision(0, 5)] = 6;
      map2[Collision(1, 3)] = 4;

      GEP_ASSERT(map2[Collision(0, 2)] == 3);
      GEP_ASSERT(map2[Collision(0, 5)] == 6);
      GEP_ASSERT(map2[Collision(1, 3)] == 4);
  }
  GEP_ASSERT( SimpleLeakCheckingAllocator::instance().getAllocCount() > 0, "not using allocator");
  //check for memory leaks
  SimpleLeakCheckingAllocator::destroyInstance();

  {
      Hashmap<std::string, std::string, StringHashPolicy> map3;

      map3[std::string("one")] = std::string("eins");
      map3[std::string("two")] = std::string("zwei");
      map3[std::string("three")] = std::string("drei");
      map3[std::string("four")] = std::string("vier");
      map3[std::string("five")] = std::string("fünf");
      map3[std::string("six")] = std::string("sechs");

      GEP_ASSERT(map3.exists(std::string("five")));
      GEP_ASSERT(map3[std::string("five")] == std::string("fünf"));

      map3.remove(std::string("five"));
      GEP_ASSERT(map3.exists(std::string("five")) == false);

      // test the constant operator[]
      const Hashmap<std::string, std::string, StringHashPolicy>& constMap3 = map3;
      GEP_ASSERT(constMap3[std::string("three")] == std::string("drei"), "const operator [] not working correctly");
  }

  {
      LifetimeCheck::reset();
      LifetimeCheckKey::reset();
      Hashmap<LifetimeCheckKey, LifetimeCheck, StdHashPolicy, SimpleLeakCheckingAllocatorPolicy> map1;
      GEP_ASSERT(LifetimeCheck::s_constructionCount == 0, "wrong number of value elements constructed", LifetimeCheck::s_constructionCount);
      GEP_ASSERT(LifetimeCheck::s_destroyCount == 0, "wrong number of value elements destroyed", LifetimeCheck::s_destroyCount);
      GEP_ASSERT(LifetimeCheck::s_copyConstructCount == 0, "wrong number of value elements copy constructed", LifetimeCheck::s_copyConstructCount);
      GEP_ASSERT(LifetimeCheck::s_moveConstructCount == 0, "wrong number of value elements move constructed", LifetimeCheck::s_moveConstructCount);
      GEP_ASSERT(LifetimeCheck::s_copyCount == 0, "wrong number of value elements copied", LifetimeCheck::s_copyCount);
      GEP_ASSERT(LifetimeCheck::s_moveCount == 0, "wrong number of value elements moved", LifetimeCheck::s_moveCount);

      GEP_ASSERT(LifetimeCheckKey::s_constructionCount == 0, "wrong number of key elements constructed", LifetimeCheckKey::s_constructionCount);
      GEP_ASSERT(LifetimeCheckKey::s_destroyCount == 0, "wrong number of key elements destroyed", LifetimeCheckKey::s_destroyCount);
      GEP_ASSERT(LifetimeCheckKey::s_copyConstructCount == 0, "wrong number of key elements copy constructed", LifetimeCheckKey::s_copyConstructCount);
      GEP_ASSERT(LifetimeCheckKey::s_moveConstructCount == 0, "wrong number of key elements move constructed", LifetimeCheckKey::s_moveConstructCount);
      GEP_ASSERT(LifetimeCheckKey::s_copyCount == 0, "wrong number of key elements copied", LifetimeCheckKey::s_copyCount);
      GEP_ASSERT(LifetimeCheckKey::s_moveCount == 0, "wrong number of key elements moved", LifetimeCheckKey::s_moveCount);

      // test copy assignment
      {
          Hashmap<LifetimeCheckKey, LifetimeCheck, StdHashPolicy, SimpleLeakCheckingAllocatorPolicy> map2;
          map2[LifetimeCheckKey(1)] = LifetimeCheck();
          map2[LifetimeCheckKey(2)] = LifetimeCheck();
          map2[LifetimeCheckKey(3)] = LifetimeCheck();
          GEP_ASSERT(LifetimeCheck::s_constructionCount == 6, "wrong number of value elements constructed", LifetimeCheck::s_constructionCount);
          GEP_ASSERT(LifetimeCheck::s_destroyCount == 6, "wrong number of value elements destroyed", LifetimeCheck::s_destroyCount);
          GEP_ASSERT(LifetimeCheck::s_copyConstructCount == 3, "wrong number of value elements copy constructed", LifetimeCheck::s_copyConstructCount);
          GEP_ASSERT(LifetimeCheck::s_moveConstructCount == 0, "wrong number of value elements move constructed", LifetimeCheck::s_moveConstructCount);
          GEP_ASSERT(LifetimeCheck::s_copyCount == 0, "wrong number of value elements copied", LifetimeCheck::s_copyCount);
          GEP_ASSERT(LifetimeCheck::s_moveCount == 3, "wrong number of value elements moved", LifetimeCheck::s_moveCount);

          GEP_ASSERT(LifetimeCheckKey::s_constructionCount == 3, "wrong number of key elements constructed", LifetimeCheckKey::s_constructionCount);
          GEP_ASSERT(LifetimeCheckKey::s_destroyCount == 3, "wrong number of key elements destroyed", LifetimeCheckKey::s_destroyCount);
          GEP_ASSERT(LifetimeCheckKey::s_copyConstructCount == 3, "wrong number of key elements copy constructed", LifetimeCheckKey::s_copyConstructCount);
          GEP_ASSERT(LifetimeCheckKey::s_moveConstructCount == 0, "wrong number of key elements move constructed", LifetimeCheckKey::s_moveConstructCount);
          GEP_ASSERT(LifetimeCheckKey::s_copyCount == 0, "wrong number of key elements copied", LifetimeCheckKey::s_copyCount);
          GEP_ASSERT(LifetimeCheckKey::s_moveCount == 0, "wrong number of key elements moved", LifetimeCheckKey::s_moveCount);

          map1 = map2;
          GEP_ASSERT(LifetimeCheck::s_constructionCount == 6, "wrong number of value elements constructed", LifetimeCheck::s_constructionCount);
          GEP_ASSERT(LifetimeCheck::s_destroyCount == 6, "wrong number of value elements destroyed", LifetimeCheck::s_destroyCount);
          GEP_ASSERT(LifetimeCheck::s_copyConstructCount == 6, "wrong number of value elements copy constructed", LifetimeCheck::s_copyConstructCount);
          GEP_ASSERT(LifetimeCheck::s_moveConstructCount == 0, "wrong number of value elements move constructed", LifetimeCheck::s_moveConstructCount);
          GEP_ASSERT(LifetimeCheck::s_copyCount == 0, "wrong number of value elements copied", LifetimeCheck::s_copyCount);
          GEP_ASSERT(LifetimeCheck::s_moveCount == 3, "wrong number of value elements moved", LifetimeCheck::s_moveCount);

          GEP_ASSERT(LifetimeCheckKey::s_constructionCount == 3, "wrong number of key elements constructed", LifetimeCheckKey::s_constructionCount);
          GEP_ASSERT(LifetimeCheckKey::s_destroyCount == 3, "wrong number of key elements destroyed", LifetimeCheckKey::s_destroyCount);
          GEP_ASSERT(LifetimeCheckKey::s_copyConstructCount == 6, "wrong number of key elements copy constructed", LifetimeCheckKey::s_copyConstructCount);
          GEP_ASSERT(LifetimeCheckKey::s_moveConstructCount == 0, "wrong number of key elements move constructed", LifetimeCheckKey::s_moveConstructCount);
          GEP_ASSERT(LifetimeCheckKey::s_copyCount == 0, "wrong number of key elements copied", LifetimeCheckKey::s_copyCount);
          GEP_ASSERT(LifetimeCheckKey::s_moveCount == 0, "wrong number of key elements moved", LifetimeCheckKey::s_moveCount);
      }
      GEP_ASSERT(LifetimeCheck::s_constructionCount == 6, "wrong number of value elements constructed", LifetimeCheck::s_constructionCount);
      GEP_ASSERT(LifetimeCheck::s_destroyCount == 9, "wrong number of value elements destroyed", LifetimeCheck::s_destroyCount);
      GEP_ASSERT(LifetimeCheck::s_copyConstructCount == 6, "wrong number of value elements copy constructed", LifetimeCheck::s_copyConstructCount);
      GEP_ASSERT(LifetimeCheck::s_moveConstructCount == 0, "wrong number of value elements move constructed", LifetimeCheck::s_moveConstructCount);
      GEP_ASSERT(LifetimeCheck::s_copyCount == 0, "wrong number of value elements copied", LifetimeCheck::s_copyCount);
      GEP_ASSERT(LifetimeCheck::s_moveCount == 3, "wrong number of value elements moved", LifetimeCheck::s_moveCount);

      GEP_ASSERT(LifetimeCheckKey::s_constructionCount == 3, "wrong number of key elements constructed", LifetimeCheckKey::s_constructionCount);
      GEP_ASSERT(LifetimeCheckKey::s_destroyCount == 6, "wrong number of key elements destroyed", LifetimeCheckKey::s_destroyCount);
      GEP_ASSERT(LifetimeCheckKey::s_copyConstructCount == 6, "wrong number of key elements copy constructed", LifetimeCheckKey::s_copyConstructCount);
      GEP_ASSERT(LifetimeCheckKey::s_moveConstructCount == 0, "wrong number of key elements move constructed", LifetimeCheckKey::s_moveConstructCount);
      GEP_ASSERT(LifetimeCheckKey::s_copyCount == 0, "wrong number of key elements copied", LifetimeCheckKey::s_copyCount);
      GEP_ASSERT(LifetimeCheckKey::s_moveCount == 0, "wrong number of key elements moved", LifetimeCheckKey::s_moveCount);

      GEP_ASSERT(map1.count() == 3);
      GEP_ASSERT(map1.exists(LifetimeCheckKey(1)));
      GEP_ASSERT(map1.exists(LifetimeCheckKey(2)));
      GEP_ASSERT(map1.exists(LifetimeCheckKey(3)));

      //check self assignment
      map1 = map1;

      GEP_ASSERT(map1.count() == 3);
      GEP_ASSERT(map1.exists(LifetimeCheckKey(1)));
      GEP_ASSERT(map1.exists(LifetimeCheckKey(2)));
      GEP_ASSERT(map1.exists(LifetimeCheckKey(3)));

      //Test move assignment
      {
          Hashmap<LifetimeCheckKey, LifetimeCheck, StdHashPolicy, SimpleLeakCheckingAllocatorPolicy> map2;
          map2 = std::move(map1);

          GEP_ASSERT(LifetimeCheck::s_constructionCount == 6, "wrong number of value elements constructed", LifetimeCheck::s_constructionCount);
          GEP_ASSERT(LifetimeCheck::s_destroyCount == 9, "wrong number of value elements destroyed", LifetimeCheck::s_destroyCount);
          GEP_ASSERT(LifetimeCheck::s_copyConstructCount == 6, "wrong number of value elements copy constructed", LifetimeCheck::s_copyConstructCount);
          GEP_ASSERT(LifetimeCheck::s_moveConstructCount == 0, "wrong number of value elements move constructed", LifetimeCheck::s_moveConstructCount);
          GEP_ASSERT(LifetimeCheck::s_copyCount == 0, "wrong number of value elements copied", LifetimeCheck::s_copyCount);
          GEP_ASSERT(LifetimeCheck::s_moveCount == 3, "wrong number of value elements moved", LifetimeCheck::s_moveCount);

          GEP_ASSERT(LifetimeCheckKey::s_constructionCount == 9, "wrong number of key elements constructed", LifetimeCheckKey::s_constructionCount);
          GEP_ASSERT(LifetimeCheckKey::s_destroyCount == 12, "wrong number of key elements destroyed", LifetimeCheckKey::s_destroyCount);
          GEP_ASSERT(LifetimeCheckKey::s_copyConstructCount == 6, "wrong number of key elements copy constructed", LifetimeCheckKey::s_copyConstructCount);
          GEP_ASSERT(LifetimeCheckKey::s_moveConstructCount == 0, "wrong number of key elements move constructed", LifetimeCheckKey::s_moveConstructCount);
          GEP_ASSERT(LifetimeCheckKey::s_copyCount == 0, "wrong number of key elements copied", LifetimeCheckKey::s_copyCount);
          GEP_ASSERT(LifetimeCheckKey::s_moveCount == 0, "wrong number of key elements moved", LifetimeCheckKey::s_moveCount);

          GEP_ASSERT(map1.count() == 0);
          GEP_ASSERT(!map1.exists(LifetimeCheckKey(1)));
          GEP_ASSERT(!map1.exists(LifetimeCheckKey(2)));
          GEP_ASSERT(!map1.exists(LifetimeCheckKey(3)));

          GEP_ASSERT(map2.count() == 3);
          GEP_ASSERT(map2.exists(LifetimeCheckKey(1)));
          GEP_ASSERT(map2.exists(LifetimeCheckKey(2)));
          GEP_ASSERT(map2.exists(LifetimeCheckKey(3)));

          /// test move constructor
          {
              Hashmap<LifetimeCheckKey, LifetimeCheck, StdHashPolicy, SimpleLeakCheckingAllocatorPolicy> map3(std::move(map2));
              GEP_ASSERT(LifetimeCheck::s_constructionCount == 6, "wrong number of value elements constructed", LifetimeCheck::s_constructionCount);
              GEP_ASSERT(LifetimeCheck::s_destroyCount == 9, "wrong number of value elements destroyed", LifetimeCheck::s_destroyCount);
              GEP_ASSERT(LifetimeCheck::s_copyConstructCount == 6, "wrong number of value elements copy constructed", LifetimeCheck::s_copyConstructCount);
              GEP_ASSERT(LifetimeCheck::s_moveConstructCount == 0, "wrong number of value elements move constructed", LifetimeCheck::s_moveConstructCount);
              GEP_ASSERT(LifetimeCheck::s_copyCount == 0, "wrong number of value elements copied", LifetimeCheck::s_copyCount);
              GEP_ASSERT(LifetimeCheck::s_moveCount == 3, "wrong number of value elements moved", LifetimeCheck::s_moveCount);

              GEP_ASSERT(LifetimeCheckKey::s_constructionCount == 15, "wrong number of key elements constructed", LifetimeCheckKey::s_constructionCount);
              GEP_ASSERT(LifetimeCheckKey::s_destroyCount == 18, "wrong number of key elements destroyed", LifetimeCheckKey::s_destroyCount);
              GEP_ASSERT(LifetimeCheckKey::s_copyConstructCount == 6, "wrong number of key elements copy constructed", LifetimeCheckKey::s_copyConstructCount);
              GEP_ASSERT(LifetimeCheckKey::s_moveConstructCount == 0, "wrong number of key elements move constructed", LifetimeCheckKey::s_moveConstructCount);
              GEP_ASSERT(LifetimeCheckKey::s_copyCount == 0, "wrong number of key elements copied", LifetimeCheckKey::s_copyCount);
              GEP_ASSERT(LifetimeCheckKey::s_moveCount == 0, "wrong number of key elements moved", LifetimeCheckKey::s_moveCount);

              GEP_ASSERT(map2.count() == 0);
              GEP_ASSERT(!map2.exists(LifetimeCheckKey(1)));
              GEP_ASSERT(!map2.exists(LifetimeCheckKey(2)));
              GEP_ASSERT(!map2.exists(LifetimeCheckKey(3)));

              GEP_ASSERT(map3.count() == 3);
              GEP_ASSERT(map3.exists(LifetimeCheckKey(1)));
              GEP_ASSERT(map3.exists(LifetimeCheckKey(2)));
              GEP_ASSERT(map3.exists(LifetimeCheckKey(3)));

              //test copy constructor
              {
                  Hashmap<LifetimeCheckKey, LifetimeCheck> map4(map3);
                  GEP_ASSERT(LifetimeCheck::s_constructionCount == 6, "wrong number of value elements constructed", LifetimeCheck::s_constructionCount);
                  GEP_ASSERT(LifetimeCheck::s_destroyCount == 9, "wrong number of value elements destroyed", LifetimeCheck::s_destroyCount);
                  GEP_ASSERT(LifetimeCheck::s_copyConstructCount == 9, "wrong number of value elements copy constructed", LifetimeCheck::s_copyConstructCount);
                  GEP_ASSERT(LifetimeCheck::s_moveConstructCount == 0, "wrong number of value elements move constructed", LifetimeCheck::s_moveConstructCount);
                  GEP_ASSERT(LifetimeCheck::s_copyCount == 0, "wrong number of value elements copied", LifetimeCheck::s_copyCount);
                  GEP_ASSERT(LifetimeCheck::s_moveCount == 3, "wrong number of value elements moved", LifetimeCheck::s_moveCount);

                  GEP_ASSERT(LifetimeCheckKey::s_constructionCount == 21, "wrong number of key elements constructed", LifetimeCheckKey::s_constructionCount);
                  GEP_ASSERT(LifetimeCheckKey::s_destroyCount == 24, "wrong number of key elements destroyed", LifetimeCheckKey::s_destroyCount);
                  GEP_ASSERT(LifetimeCheckKey::s_copyConstructCount == 9, "wrong number of key elements copy constructed", LifetimeCheckKey::s_copyConstructCount);
                  GEP_ASSERT(LifetimeCheckKey::s_moveConstructCount == 0, "wrong number of key elements move constructed", LifetimeCheckKey::s_moveConstructCount);
                  GEP_ASSERT(LifetimeCheckKey::s_copyCount == 0, "wrong number of key elements copied", LifetimeCheckKey::s_copyCount);
                  GEP_ASSERT(LifetimeCheckKey::s_moveCount == 0, "wrong number of key elements moved", LifetimeCheckKey::s_moveCount);

                  GEP_ASSERT(map4.count() == 3);
                  GEP_ASSERT(map4.exists(LifetimeCheckKey(1)));
                  GEP_ASSERT(map4.exists(LifetimeCheckKey(2)));
                  GEP_ASSERT(map4.exists(LifetimeCheckKey(3)));

                  GEP_ASSERT(map3.count() == 3);
                  GEP_ASSERT(map3.exists(LifetimeCheckKey(1)));
                  GEP_ASSERT(map3.exists(LifetimeCheckKey(2)));
                  GEP_ASSERT(map3.exists(LifetimeCheckKey(3)));
              }
          }
      }
      //check for memory leaks
      SimpleLeakCheckingAllocator::destroyInstance();
  }
}

GEP_UNITTEST_TEST(Container, queue)
{
  {
    Queue<int> q1;

    for(int i=0; i<5; i++)
      q1.append(i);

    for(int i=0; i<5; i++)
    {
      GEP_ASSERT(q1.take() == i, "queue not working correctly");
    }
  }

  {
    Queue<int> q2;
    for(int i=0; i<3; i++)
    {
      q2.append(i);
    }

    for(int i=0; i<3; i++)
    {
      GEP_ASSERT(i == q2.take());
    }

    for(int i=0; i<10; i++)
    {
      q2.append(i);
    }

    for(int i=0; i<10; i++)
    {
      GEP_ASSERT(q2.take() == i);
    }
  }
}
