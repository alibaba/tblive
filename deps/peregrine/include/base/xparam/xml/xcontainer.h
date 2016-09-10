#pragma once
#ifndef __XCONTAINER_H_0261C364_E6DD_4456_B6E1_09519AA9D67D__
#define __XCONTAINER_H_0261C364_E6DD_4456_B6E1_09519AA9D67D__

#include <vector>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

template <typename T, typename P, typename K, typename LT, typename LTK, typename EK>
class xcontainer
{
public:
	struct PointerLT
	{
		inline bool operator()(T const* p1, T const* p2) const
		{
			return LT()(*p1, *p2);
		}
	};
	inline xcontainer() : m_count(0) 
	{
		concept_T_must_have_specified_fields();
	}
	inline void concept_T_must_have_specified_fields()
	{
//		offsetof(T, m_parent);
//		offsetof(T, m_prev);
//		offsetof(T, m_next);
	}
	inline ~xcontainer()
	{
		clear();
		clear_index();
	}
	inline size_t count() const
	{
		return m_count;
	}
	inline void push_front(scoped_refptr<T> const& p, P* pa)
	{
		assert(p && pa && (!p->m_parent) && (!p->m_prev) && (!p->m_next));
		if (!p || !pa || p->m_parent)
			return;
		p->m_prev = NULL;
		p->m_next = m_head;
		if (m_head)
			m_head->m_prev = p;
		else
			m_tail = p;
		m_head = p;
		++m_count;
		p->m_parent = pa;
		if (m_index)
		{
			typename std::vector<T*>::iterator iter = lower_bound(
				m_index->begin(), m_index->end(), p.get(), PointerLT());
			m_index->insert(iter, p.get());
		}
	}
	inline void push_back(scoped_refptr<T> const& p, P* pa)
	{
		assert(p && pa && (!p->m_parent) && (!p->m_prev) && (!p->m_next));
		if (!p || !pa || p->m_parent)
			return;
		p->m_prev = m_tail;
		p->m_next = NULL;
		if (m_tail)
			m_tail->m_next = p;
		else
			m_head = p;
		m_tail = p;
		++m_count;
		p->m_parent = pa;
		if (m_index)
		{
			typename std::vector<T*>::iterator iter = upper_bound(
				m_index->begin(), m_index->end(), p.get(), PointerLT());
			m_index->insert(iter, p.get());
		}
	}
	inline void insert_before(scoped_refptr<T> const& before, scoped_refptr<T> const& p, P* pa)
	{
		if (!before)
		{
			push_front(p, pa);
			return;
		}
		assert(p && pa && (!p->m_parent) && (!p->m_prev) && (!p->m_next));
		if (!p || !pa || p->m_parent)
			return;
		p->m_prev = before->m_prev;
		p->m_next = before;
		if (before->m_prev)
			before->m_prev->m_next = p;
		before->m_prev = p;
		++m_count;
		p->m_parent = pa;
		if (m_index)
		{
			if ( PointerLT()( before.get(), p.get() ) )
			{
				typename std::vector<T*>::iterator iter = lower_bound(
					m_index->begin(), m_index->end(), p.get(), PointerLT());
				m_index->insert(iter, p.get());
			}
			else if ( PointerLT()( p.get(), before.get() ) )
			{
				typename std::vector<T*>::iterator iter = upper_bound(
					m_index->begin(), m_index->end(), p.get(), PointerLT());
				m_index->insert(iter, p.get());
			}
			else
			{
				std::pair<typename std::vector<T*>::iterator, typename std::vector<T*>::iterator> result =
					std::equal_range(m_index->begin(), m_index->end(), before.get(), PointerLT());
				for (typename std::vector<T*>::iterator iter = result.first; iter != result.second; ++ iter)
				{
					if (*iter == before.get())
					{
						m_index->insert(iter, p.get());
						break;
					}
				}
			}
		}
	}
	inline void insert_after(scoped_refptr<T> const& after, scoped_refptr<T> const& p, P* pa)
	{
		if (!after)
		{
			push_back(p, pa);
			return;
		}
		assert(p && pa && (!p->m_parent) && (!p->m_prev) && (!p->m_next));
		if (!p || !pa || p->m_parent)
			return;
		p->m_next = after->m_next;
		p->m_prev = after;
		if (after->m_next)
			after->m_next->m_prev = p;
		after->m_next = p;
		++m_count;
		p->m_parent = pa;
		if (m_index)
		{
			if ( PointerLT()( after.get(), p.get() ) )
			{
				typename std::vector<T*>::iterator iter = lower_bound(
					m_index->begin(), m_index->end(), p.get(), PointerLT());
				m_index->insert(iter, p.get());
			}
			else if ( PointerLT()( p.get(), after.get() ) )
			{
				typename std::vector<T*>::iterator iter = upper_bound(
					m_index->begin(), m_index->end(), p.get(), PointerLT());
				m_index->insert(iter, p.get());
			}
			else
			{
				std::pair<typename std::vector<T*>::iterator, typename std::vector<T*>::iterator> result =
					std::equal_range(m_index->begin(), m_index->end(), after.get(), PointerLT());
				for (typename std::vector<T*>::iterator iter = result.first; iter != result.second; ++ iter)
				{
					if (*iter == after.get())
					{
						m_index->insert(++iter, p.get());
						break;
					}
				}
			}
		}
	}
	inline void remove(scoped_refptr<T> const& p, P* pa)
	{
		assert(p && pa && p->m_parent == pa);
		if (!p || !pa || p->m_parent != pa)
			return;
		if (p->m_next)
			p->m_next->m_prev = p->m_prev;
		if (p->m_prev)
			p->m_prev->m_next = p->m_next;
		if (m_head == p)
			m_head = p->m_next;
		if (m_tail == p)
			m_tail = p->m_prev;
		p->m_next = NULL;
		p->m_prev = NULL;
		--m_count;
		p->m_parent = NULL;

		if (m_index)
		{
			std::pair<typename std::vector<T*>::iterator, typename std::vector<T*>::iterator> result =
				std::equal_range(m_index->begin(), m_index->end(), p.get(), PointerLT());
			for (typename std::vector<T*>::iterator iter = result.first; iter != result.second; ++ iter)
			{
				if (*iter == p.get())
				{
					m_index->erase(iter);
					break;
				}
			}
		}
	}
	inline void clear()
	{
		if (!m_count)
			return;
		for (scoped_refptr<T> p = m_head; p; )
		{
			scoped_refptr<T> n = p->m_next;
			p->m_next = NULL;
			p->m_prev = NULL;
			p->m_parent = NULL;
			p = n;
		}
		m_head = NULL;
		m_tail = NULL;
		m_count = 0;

		if (m_index)
			m_index->clear();
	}
	inline scoped_refptr<T> first() const
	{
		return m_head;
	}
	inline scoped_refptr<T> first(K const& key, size_t position) const
	{
		if (m_index)
		{
			typename std::vector<T*>::const_iterator iter = std::lower_bound(m_index->begin(), m_index->end(), key, LTK());
			if (m_index->end() > iter + position && EK()(**(iter + position), key))
				return scoped_refptr<T>(*(iter + position));
			else
				return scoped_refptr<T>();
		}
		else
		{
			size_t counter = 0;
			for (T* p = m_head.get(); p; p = p->m_next.get())
			{
				if (EK()(*p, key))
				{
					if(counter == position)
						return scoped_refptr<T>(p);
					++counter;
				}
			}
			return scoped_refptr<T>();
		}
	}

	inline scoped_refptr<T> last() const
	{
		return m_tail;
	}
	inline scoped_refptr<T> last(K const& key, size_t position) const
	{
		if (m_index)
		{
			typename std::vector<T*>::const_iterator iter = std::upper_bound(m_index->begin(), m_index->end(), key, LTK());
			if (iter - position > m_index->begin() && EK()(**(iter - position - 1), key))
				return scoped_refptr<T>(*(iter - position - 1));
			else
				return scoped_refptr<T>();
		}
		else
		{
			size_t counter = 0;
			for (T* p = m_tail.get(); p; p = p->m_prev.get())
			{
				if (EK()(*p, key))
				{
					if(counter == position)
						return scoped_refptr<T>(p);
					++counter;
				}
			}
			return scoped_refptr<T>();
		}
	}
	inline void build_index()
	{
		if (m_index)
			return;
		m_index.reset(new std::vector<T*>());
		if (m_count)
		{
			m_index->reserve(m_count);
			for (T* p = m_head.get(); p; p = p->m_next.get())
				m_index->push_back(p);
			std::stable_sort(m_index->begin(), m_index->end(), PointerLT());
		}
	}
	inline void clear_index()
	{
		if (!m_index)
			return;
		m_index.reset();
	}
	inline bool has_index() const
	{
		return m_index;
	}
private:
	scoped_refptr<T> m_head;
	scoped_refptr<T> m_tail;
	size_t m_count;
	scoped_ptr< std::vector<T*> > m_index;
};



#endif//__XCONTAINER_H_0261C364_E6DD_4456_B6E1_09519AA9D67D__
