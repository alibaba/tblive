#pragma once
#ifndef __XSTR_H_0261C364_E6DD_4456_B6E1_09519AA9D67D__
#define __XSTR_H_0261C364_E6DD_4456_B6E1_09519AA9D67D__

#include <memory.h>
#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#pragma intrinsic(memcpy, memcmp, memset)
#endif

#ifndef NULL
#define NULL 0
#endif

namespace xstr
{
	
	template <typename XTCHAR>
	class xstring
	{
	public:
		inline static void normalize_size(const XTCHAR* s, size_t& len)
		{
			if (s == NULL)
				len = 0;
			else if (len == (size_t)-1)
			{
				const XTCHAR *eos = s;
				while( *eos++ );
				len =  eos - s - 1 ;
			}
		}
		inline static size_t allocSize()
		{
			return s_allocSize;
		}
		xstring() : m(NULL) {}
		inline xstring(const xstring& rhs) : m(NULL)
		{
			size_t len = rhs.length();
			if (len == 0)
				return;
			m = create_sdata(calc_min_cap(len));
			copys(rhs.data(), rhs.length());
		}
		inline xstring(const XTCHAR* s, size_t len) : m(NULL)
		{
			normalize_size(s, len);
			if (len == 0)
				return;
			m = create_sdata(calc_min_cap(len));
			copys(s, len);
		}
		inline ~xstring()
		{
			destroy_sdata(m);
		}
		inline void swap(xstring& src)
		{
			if (m != src.m)
			{
				sdata* tmp = m;
				m = src.m;
				src.m = tmp;
			}
		}
		inline xstring& operator=(const xstring& rhs)
		{
			if (m == rhs.m)
				return *this;
			return assign(rhs.data(), rhs.length());
		}
		inline bool operator==(const xstring& rhs) const
		{
			if (m == rhs.m)
				return true;
			if (length() != rhs.length())
				return false;
			if (empty())
				return true;
			return memcmp(m->s, rhs.data(), length() * sizeof(XTCHAR)) == 0;
		}
		inline bool operator!=(const xstring& rhs) const
		{
			return !operator==(rhs);
		}
		inline xstring& operator+=(const xstring& rhs)
		{
			return append(rhs.data(), rhs.length());
		}
		inline xstring& assign(const XTCHAR* s, size_t len)
		{
			normalize_size(s, len);
			if (len == 0)
			{
				clear();
				return *this;
			}

			if (m->s == s && length() == len)
				return *this;

			size_t oldcap = cap();
			if (oldcap < len || oldcap > calc_max_cap(len)) // 须重分配
			{
				sdata* om = m;
				m = create_sdata(calc_min_cap(len));
				copys(s, len);
				destroy_sdata(om);
			}
			else
				copys(s, len);
			return *this;
		}
		inline xstring& assign(XTCHAR c)
		{
			size_t oldcap = cap();
			if (oldcap < 1 || oldcap > MIN_LEN)
			{
				destroy_sdata(m);
				m = create_sdata(MIN_LEN);
			}
			m->s[0] = c;
			m->s[1] = 0;
			m->len = 1;
			return *this;
		}
		inline bool equal(const XTCHAR* s, size_t len) const
		{
			normalize_size(s, len);
			if (length() != len)
				return false;
			if (empty())
				return true;
			if (m->s == s)
				return true;
			return memcmp(m->s, s, len * sizeof(XTCHAR)) == 0;
		}
		inline static int compare(const XTCHAR* s, size_t slen, const XTCHAR* d, size_t dlen)
		{
			normalize_size(s, slen);
			normalize_size(d, dlen);
			if (s == d)
			{
				if (slen < dlen)
					return -1;
				if (slen > dlen)
					return 1;
				return 0;
			}
			size_t minlen = slen < dlen ? slen : dlen;
			int ret = memcmp(s, d, minlen * sizeof(XTCHAR));
			if (slen > dlen)
				return ret >= 0 ? 1 : -1;
			if (slen < dlen)
				return ret > 0 ? 1 : -1;
			return ret;
		}
		inline int compare(const XTCHAR* s, size_t len) const
		{
			return compare(data(), length(), s, len);
		}
		inline xstring& append(const XTCHAR* s, size_t len)
		{
			normalize_size(s, len);
			if (len == 0)
				return *this;

			if (m)
			{
				size_t oldlen = length();
				size_t oldcap = cap();
				size_t newlen = oldlen + len;
				if (oldcap < newlen) // 须重分配
				{
					sdata* om = m;
					m = create_sdata(calc_new_cap(oldlen, newlen));
					memcpy(m->s, om->s, oldlen * sizeof(XTCHAR));
					copyz(m->s + oldlen, s, len);
					destroy_sdata(om);
				}
				else
				{
					copyz(m->s + oldlen, s, len);
				}
				m->len = newlen;
			}
			else
			{
				m = create_sdata(calc_min_cap(len));
				copys(s, len);
			}
			return *this;
		}
		inline xstring& append(XTCHAR c)
		{
			if (m)
			{
				size_t oldcap = cap();
				size_t oldlen = length();
				if (oldcap < oldlen + 1) // 须重分配
				{
					sdata* om = m;
					m = create_sdata(calc_max_cap(oldlen + 1));
					memcpy(m->s, om->s, oldlen * sizeof(XTCHAR));
					destroy_sdata(om);
				}
				m->s[oldlen] = c;
				m->s[oldlen + 1] = 0;
				m->len = oldlen + 1;
			}
			else
			{
				m = create_sdata(MIN_LEN);
				m->s[0] = c;
				m->s[1] = 0;
				m->len = 1;
			}
			return *this;
		}
		inline xstring& insert(XTCHAR const* pos, XTCHAR const* s, size_t len)
		{
			normalize_size(s, len);
			if (len == 0)
				return *this;
			if (m)
			{
				size_t oldlen = length();
				size_t oldcap = cap();
				size_t newlen = oldlen + len;
				if (pos == NULL)
					pos = m->s + oldlen;
				else
					assert(pos >= m->s && pos <= m->s + oldlen);
				if (oldcap < newlen) // 须重分配
				{
					sdata* om = m;
					m = create_sdata(calc_new_cap(oldlen, newlen));
					memcpy(m->s, om->s, (pos - om->s) * sizeof(XTCHAR));
					memcpy(m->s + pos - om->s, s, len * sizeof(XTCHAR));
					memcpy(m->s + pos - om->s + len, pos, (om->s + oldlen - pos + 1) * sizeof(XTCHAR));
					destroy_sdata(om);
				}
				else
				{
					if (s + len > pos && m + m->cap > s) // [s, s + len] 竟然和 [pos, m + m->cap]互相覆盖 
					{
						XTCHAR* buf = new XTCHAR[len];
						memcpy(buf, s, len);
						memcpy(pos + len, pos, (m->s + oldlen - pos + 1) * sizeof(XTCHAR));
						memcpy(pos, buf, len * sizeof(XTCHAR));
						delete[] buf;
					}
					else
					{
						memcpy(pos + len, pos, (m->s + oldlen - pos + 1) * sizeof(XTCHAR));
						memcpy(pos, s, len * sizeof(XTCHAR));
					}
				}
				m->len = newlen;
			}
			else
			{
				m = create_sdata(calc_min_cap(len));
				copys(s, len);
			}
			return *this;
		}
//		inline xstring& insert(XTCHAR const* pos, XTCHAR c)
//		{
//			if (m)
//			{
//				size_t oldlen = length();
//				size_t oldcap = cap();
//				size_t newlen = oldlen + 1;
//				if (pos == NULL)
//					pos = m->s + oldlen;
//				else
//					assert(pos >= m->s && pos <= m->s + oldlen);
//				if (oldcap < newlen) // 须重分配
//				{
//					sdata* om = m;
//					m = create_sdata(calc_max_cap(newlen));
//					memcpy(m->s, om->s, (pos - om->s) * sizeof(XTCHAR));
//					m->s[pos - om->s] = c;
//					memcpy(m->s + pos - om->s + 1, pos, (om->s + oldlen - pos + 1) * sizeof(XTCHAR));
//					destroy_sdata(om);
//				}
//				else
//				{
//					memcpy(pos + len, pos, (m->s + oldlen - pos + 1) * sizeof(XTCHAR));
//					*pos = c;
//				}
//				m->len = newlen;
//			}
//			else
//			{
//				m = create_sdata(MIN_LEN);
//				m->s[0] = c;
//				m->s[1] = 0;
//				m->len = 1;
//			}
//			return *this;
//		}
//		inline void reserve(size_t cap)
//		{
//			size_t oldcap = cap();
//			if (cap <= oldcap)
//				return;
//			sdata* om = m;
//			m = create_sdata(cap);
//			copys(om->s, om->len);
//			destroy_sdata(om);
//		}
		inline void resize(size_t len)
		{
			size_t oldlen = length();
			if (oldlen == len)
				return;
			size_t oldcap = cap();

			if (len > oldlen)
			{
				if (m)
				{
					if (oldcap < len ) // 须重分配
					{
						sdata* om = m;
						m = create_sdata(calc_new_cap(oldlen, len));
						memcpy(m->s, om->s, oldlen);
						destroy_sdata(om);
					}
					memset(m->s + oldlen, 0, (len - oldlen + 1) * sizeof(XTCHAR));
				}
				else
				{
					m = m = create_sdata(calc_min_cap(len));
					memset(m->s, 0, (len + 1) * sizeof(XTCHAR));
				}
				m->len = len;
			}
			else
			{
				if (len == 0)
				{
					clear();
				}
				else
				{
					if (oldcap > calc_max_cap(len)) // 须重分配
					{
						sdata* om = m;
						m = create_sdata(calc_min_cap(len));
						copys(om->s, len);
						destroy_sdata(om);
					}
					else
						m->s[len] = 0;
					m->len = len;
				}
			}
		}
		inline void clear()
		{
			destroy_sdata(m);
			m = NULL;
		}
		inline void trim()
		{
			XTCHAR const* eos = end();
			XTCHAR const* p = begin();
			while(p < eos)
			{
				if (*p == 9 || *p == 10 || *p == 13 || *p == 32)
					++p;
				else
					break;
			}
			XTCHAR const* news = p;
			p = eos - 1;
			while (p > news)
			{
				if (*p == 9 || *p == 10 || *p == 13 || *p == 32)
					--p;
				else
					break;
			}
			size_t newlen = p + 1 - news;
			assign(news, newlen);
		}
		inline void compact()
		{
			if (m && m->cap > m->len)
			{
				xstring<XTCHAR> tmp(m->s, m->len);
				swap(tmp);
			}
		}

		inline size_t length() const
		{
			return m ? m->len : 0;
		}
		inline size_t size() const
		{
			return length();
		}
		inline size_t cap() const
		{
			return m ? m->cap : 0;
		}
		inline bool empty() const
		{
			return length() == 0;
		}
		inline const XTCHAR* c_str() const
		{
			const static XTCHAR empty_str[1] = {0};
			return m ? m->s : empty_str;
		}
		inline const XTCHAR* data() const
		{
			return c_str();
		}
		inline const XTCHAR* begin() const
		{
			return c_str();
		}
		inline const XTCHAR* end() const
		{
			return c_str() + length();
		}
		inline const XTCHAR& operator[](size_t index) const
		{
			return m->s[index];
		}
		inline XTCHAR& operator[](size_t index)
		{
			return m->s[index];
		}
	private:
		struct sdata
		{
			long rcounter;
			size_t cap;
			size_t len;
			XTCHAR s[1];
		};
		inline static sdata* create_sdata(size_t cap)
		{
			size_t news = sizeof(sdata) + cap * sizeof(XTCHAR);
			sdata* sd = reinterpret_cast<sdata*>(::operator new(news));
			s_allocSize += news;
			sd->rcounter = 1;
			sd->cap = cap;
			sd->len = 0;
			sd->s[0] = 0;
			return sd;
		}
		inline static void destroy_sdata(sdata* sd)
		{
			if (sd == NULL)
				return;
			s_allocSize -= sizeof(sdata) + sd->cap * sizeof(XTCHAR);
			::operator delete(sd);
		}
		inline static size_t calc_max_cap(size_t len)
		{
			return (len > 0 && len < MIN_LEN) ? MIN_LEN : len + (len >> 1); // 最小是MIN_LEN，否则是1.5倍
		}
		inline static size_t calc_min_cap(size_t len)
		{
			return len < MIN_LEN ? MIN_LEN : len; // 最小是MIN_LEN，否则是len
		}
		inline static size_t calc_new_cap(size_t oldlen, size_t newlen)
		{
			size_t old_max_cap = calc_max_cap(oldlen);
			size_t new_min_cap = calc_min_cap(newlen);
			return old_max_cap > new_min_cap ? old_max_cap : new_min_cap;
		}
		inline static void copyz(XTCHAR* dst, const XTCHAR* src, size_t len)
		{
			memcpy(dst, src, len * sizeof(XTCHAR));
			dst[len] = 0;
		}
		inline void copys(const XTCHAR* src, size_t len)
		{
			m->len = len;
			copyz(m->s, src, len);
		}
		enum {MIN_LEN = 8};
		sdata* m;
		static size_t s_allocSize;
	};

	template <typename XTCHAR>
	size_t xstring<XTCHAR>::s_allocSize = 0;

}

#endif//__XSTR_H_0261C364_E6DD_4456_B6E1_09519AA9D67D__
