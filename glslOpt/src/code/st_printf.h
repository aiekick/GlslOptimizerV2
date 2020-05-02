#ifndef __ST_PRINTF__H__
#define __ST_PRINTF__H__

#include <stdarg.h>
#include <string>

#include "main/macros.h"

class st
{
public:
	st() {}

	static void stprintf(std::string& buffer, const char *fmt, ...)
	{
		if (fmt)
		{
			va_list args;
			va_start(args, fmt);

			size_t size = _vscprintf(fmt, args);
			char *buf = new char[size + 1];
			size_t nsize = vsnprintf(buf, size, fmt, args);
			buffer += buf;
			delete[] buf;

			va_end(args);
		}
	}
};

/* from glsl_optimizer */
class sbuffer
{
public:
	sbuffer(void* mem_ctx)
	{
		m_Capacity = 512;
		m_Ptr = (char*)ralloc_size(mem_ctx, m_Capacity);
		m_Size = 0;
		m_Ptr[0] = 0;
	}

	~sbuffer()
	{
		ralloc_free(m_Ptr);
	}

	bool empty() const { return m_Size == 0; }

	const char* c_str() const { return m_Ptr; }

	void append(const char *fmt, ...) PRINTFLIKE(2, 3)
	{
		va_list args;
		va_start(args, fmt);
		vasprintf_append(fmt, args);
		va_end(args);
	}

	void vasprintf_append(const char *fmt, va_list args)
	{
		assert(m_Ptr != NULL);
		vasprintf_rewrite_tail(&m_Size, fmt, args);
	}

	void vasprintf_rewrite_tail(size_t *start, const char *fmt, va_list args)
	{
		assert(m_Ptr != NULL);

		size_t new_length = _vscprintf(fmt, args);
		size_t needed_length = m_Size + new_length + 1;

		if (m_Capacity < needed_length)
		{
			m_Capacity = MAX2(m_Capacity + m_Capacity / 2, needed_length);
			m_Ptr = (char*)reralloc_size(ralloc_parent(m_Ptr), m_Ptr, m_Capacity);
		}

		vsnprintf(m_Ptr + m_Size, new_length + 1, fmt, args);
		m_Size += new_length;
		assert(m_Capacity >= m_Size);
	}

private:
	char* m_Ptr;
	size_t m_Size;
	size_t m_Capacity;
};

#endif // __ST_PRINTF__H__