#ifndef INC_AL_SINGLE_RW_RING_BUFFER_HPP
#define INC_AL_SINGLE_RW_RING_BUFFER_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Passing data between a pair of threads without locking

	Author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

namespace al {

/// Lock free single-reader-single-writer ring buffer

/// Can be used to stream data safely between two threads, one being
/// a reader, one a writer. There is no locking in this ring buffer,
/// so it is ideal to pass data to and from a high priority thread
/// like an audio thread.
/// \ingroup allocore
class SingleRWRingBuffer {
public:

	/// Allocate ringbuffer
	/// Actual size rounded up to next power of 2
	SingleRWRingBuffer(size_t sz=256);

	~SingleRWRingBuffer();


	/// Get number of bytes available for writing
	size_t writeSpace() const;

	/// Get number of bytes available for reading
	size_t readSpace() const;

	/// Copy bytes into the ringbuffer
	/// \returns number of bytes copied
	size_t write(const char * src, size_t sz);

	/// Read data from the buffer and advance the read pointer
	/// \returns number of bytes copied
	size_t read(char * dst, size_t sz);

	/// Read data without advancing the read pointer
	/// \returns number of bytes copied
	size_t peek(char * dst, size_t sz);

protected:
	size_t mSize, mWrap;
	size_t mRead, mWrite;
	char * mData;
};

} // al::
#endif
