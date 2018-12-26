#pragma once
/** \file
	Defines the Pool and MyAllocator classes used for object memory managment.
*/

/** Pool is a fast allocating buffer of many objects of the same type. It is used
	for memory access optimization in the PicPainter and MyObject.
	A Pool of objects of type T can be created with a certain size. When a call
	to allocate() is made, one of those objects is returned.
	Objects in the pool can't be individually deallocated. instead, the who
	pool is discarded once there is no more need for the objects allocated in it.
	This is a really silly garbage collection mechanism.
	The pool cannot grow above its initial size. if allocate() is called more times
	then there are items available in the pool, it will return an object which
	is allocated individually outside the pool. This object is likely to be leaked
	since there is no direct indication to the user of this situation.
	The user can detect an overallocation (and possibly prevent it although this is
	not done) by comparing size() with getMaxAlloc()
	\see MyAllocator
*/
template<typename T>
class Pool
{
public:
	Pool<T>(int size = 0) :m_buffers(nullptr), m_size(size), m_next(0), m_allocated(0), m_maxAllocated(0)
	{
		if (m_size > 0)
			m_buffers = new T[m_size];
	}
	bool isNull() const { 
        return m_buffers == nullptr; 
    }
	int size() const { 
        return m_size; 
    }
	void init(int size)
	{
		if (m_buffers != nullptr)
			delete[] m_buffers;
		m_size = size;
		if (m_size > 0)
			m_buffers = new T[m_size];
		clear();
		clearMaxAlloc();
	}
	T* allocate()
	{
		++m_allocated;
		if (m_next < m_size)
			return &m_buffers[m_next++];
		else
			return new T();
	}
	void clear()
	{
	//	for(int i = 0; i < m_size; ++i) no real need to do this.
	//		m_buffers[i] = T();
		m_next = 0;
		if (m_allocated > m_maxAllocated)
			m_maxAllocated = m_allocated;
		m_allocated = 0;
	}

	void clearMaxAlloc() { m_maxAllocated = 0; }
	int getMaxAlloc() const { return m_maxAllocated; }

private:
	T* m_buffers;
	int m_size;
	int m_next; // the index of the next buffer to allocate
	int m_allocated;
	int m_maxAllocated;

};

/** MyAllocator is a helper memory allocator for MyPoint, MyPolygon and HalfEdge objects.
	It maintains three pools for the three object classes. The method checkMaxAlloc()
	can be used to detect overallocation as discussed in Pool.
	this class is used in PicPainter and MyObject in the process of creating the
	pieces displa lists and in the implementation of the polygon subdivision algorithm
	in MyObject::subdivide()
	\see Pool MyObject
*/
struct MyAllocator
{
	MyAllocator() {}
	MyAllocator(int points, int poly, int he)
		:m_pointsPool(points), m_polyPool(poly), m_hePool(he) {}
	bool isNull()
	{
		return m_pointsPool.isNull();
	}

	void init(int points, int poly, int he)
	{
		// never shrinks... 
		if (points > m_pointsPool.size())
			m_pointsPool.init(points);
		if (poly > m_polyPool.size())
			m_polyPool.init(poly);
		if (he > m_hePool.size())
			m_hePool.init(he);
	}

	void clear()
	{
		m_pointsPool.clear();
		m_polyPool.clear();
		m_hePool.clear();
	}

	void clearMaxAlloc()
	{
		m_pointsPool.clearMaxAlloc();
		m_polyPool.clearMaxAlloc();
		m_hePool.clearMaxAlloc();
	}

	void checkMaxAlloc() const
	{
		if ((m_pointsPool.getMaxAlloc() > m_pointsPool.size()) ||
			(m_polyPool.getMaxAlloc() > m_polyPool.size()) ||
			(m_hePool.getMaxAlloc() > m_hePool.size())) 
        {
			cout << "Warning! memory pools not optimal. max: ";
		}
        else {
            cout << "Pool sizes: ";
        }
        cout << " pnt=" << m_pointsPool.size() << "," << m_pointsPool.getMaxAlloc() << 
              "  poly=" << m_polyPool.size() << "," << m_polyPool.getMaxAlloc() << 
                 " he=" << m_hePool.size() << "," << m_hePool.getMaxAlloc() << endl;
	}

	Pool<MyPoint> m_pointsPool; //(671100); //168100);
	Pool<MyPolygon> m_polyPool; //(891000); // TBD - divide to levels, for each subdivision
	Pool<HalfEdge> m_hePool; //(5000);

};

