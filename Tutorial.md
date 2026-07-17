# Tutorial for STLite-LinkedHashMap

## (1): HashMap

LinkedHashMap is essentially a HashMap that implements insertion-order access to elements. Therefore, before introducing LinkedHashMap, we need to understand HashMap first.

HashMap is a data structure that uses key-value indexing. It supports insertion of key-value pairs and querying elements corresponding to keys, and the expected time complexity of these operations is linear.

### 1. Hash-related Concepts

Hashing is the process of converting arbitrary-length input into a corresponding output (usually an integer) through a certain hash algorithm. We call this output the hash value. Different inputs may be hashed to the same output, so it's impossible to uniquely determine the input value from the hash value. This situation is called a hash collision. (In this assignment, it is recommended to use `std::hash` to implement the hash function)

### 2. Using Hash Algorithm to Build a Data Structure that Supports Fast Search and Deletion

With a hash function, the most natural idea is to build an array where the array index corresponds to the hash value. Each time we insert a pair (Key, Value), we first process the Key through the hash function to get a hash value `hash`, then store the Value at position `hash mod Listsize` of the array. However, we will encounter a problem. When we encounter the hash collision problem mentioned earlier, we have no way to store two elements at the same index position of the array. Therefore, a relatively natural idea is to open a linked list for each index. If we encounter a collision, we add this element to the end of the linked list. When we query, for a given Key, we first hash it to get the hash value `hash`, then sequentially search for the element corresponding to Key in the linked list at position `hash mod Listsize`. This implements a fixed-size hash table.

### 3. Some Optimizations

What we need to implement is a well-encapsulated data structure for others to use. During development, we actually don't know the size of the data in advance, so the choice of hash table size is a very critical issue. When we choose a relatively small hash table size, the linked list length may be quite long, and query complexity will degrade. When the hash table size is relatively large, it will occupy a lot of unused space. This allows us to dynamically change the size of the hash table. First, we introduce two parameters `Capacity` and `LoadFactor`, which represent capacity and load factor, respectively representing the size of the hash table and, for a certain given capacity, the ratio of the maximum number of elements we can accept to the capacity. We can start by choosing a relatively small capacity. When the number of elements is greater than `Capacity * LoadFactor`, we increase our `Capacity`, enabling our data structure to maintain good efficiency while not occupying too much space. Specific parameters can be adjusted according to your own implemented data structure.

## (2): LinkedHashMap

LinkedHashMap needs to maintain insertion order on the basis of implementing HashMap functionality, so that we can access elements according to insertion order. The implementation of this feature is relatively simple. We just need to maintain a doubly linked list. Each time an element is added, in addition to inserting it into the HashMap, it also needs to be inserted at the end of the linked list. This way, we can access elements in linked list order to achieve this functionality.

## P.S.

The LinkedHashMap we implement is basically the same as Java's LinkedHashMap in underlying principles and algorithms. If you don't understand something about the algorithm, you can refer to the implementation of Java's LinkedHashMap.
