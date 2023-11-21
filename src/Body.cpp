#include "Body.hpp"

Body::Body(): _lastChunk(false), _length(0), _received(0), _chunkExpected(CHUNK_SIZE), _state(NO_BODY) {}

Body::~Body() {}

Body::Body(const Body &src)
{
	*this = src;
}

Body	&Body::operator=(const Body &src)
{
	if (this != &src)
	{
		this->_length = src._length;
		this->_received = src._received;
		this->_state = src._state;
	}
	return (*this);
}

static bool	isHexa(char c)
{
	return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

static unsigned int	hstoi(std::string hexa)
{
	int		result = 0;
	int		power = 1;

	for (int i = hexa.size() - 1; i >= 0; i--)
	{
		if (hexa[i] >= '0' && hexa[i] <= '9')
			result += (hexa[i] - '0') * power;
		else if (hexa[i] >= 'a' && hexa[i] <= 'f')
			result += (hexa[i] - 'a' + 10) * power;
		else if (hexa[i] >= 'A' && hexa[i] <= 'F')
			result += (hexa[i] - 'A' + 10) * power;
		power *= 16;
	}
	return (result);
}

void Body::parseLine(const std::string line)
{
	if (this->_state == NO_BODY || this->_state == FINISHED)
		return;
	if (this->_received == this->_length)
	{
		this->_state = FINISHED;
		return;
	}
	if (this->_state == CONTENT_LENGTH)
	{
		for (unsigned int i = 0; i < line.size(); i++)
		{
			this->_body.push_back(line[i]);
			this->_received++;
			if (this->_received == this->_length)
			{
				this->_state = FINISHED;
				return;
			}
		}
	}
	else
	{
		for (unsigned int i = 0; i < line.size(); i++)
		{
			if (this->_chunkExpected == CHUNK_END_R)
			{
				if (line[i] != '\r')
					throw BodyException();
				this->_chunkExpected = CHUNK_END_N;
			}
			else if (this->_chunkExpected == CHUNK_END_N)
			{
				if (line[i] != '\n')
					throw BodyException();
				if (this->_hexa.size() == 0)
					this->_chunkExpected = CHUNK_SIZE;
				else
				{
					if (this->_lastChunk)
					{
						this->_state = FINISHED;
						return;
					}
					this->_length = hstoi(this->_hexa);
					if (this->_length == 0)
					{
						this->_chunkExpected = CHUNK_END_R;
						this->_lastChunk = true;
					}
					this->_received = 0;
					this->_chunkExpected = CHUNK_DATA;
					this->_hexa.clear();
				}
			}
			else if (this->_chunkExpected == CHUNK_SIZE)
			{
				if (line[i] == '\r')
				{
					this->_chunkExpected = CHUNK_END_N;
					continue;
				}
				if (!isHexa(line[i]))
					throw BodyException();
				this->_hexa.push_back(line[i]);
			}
			else if (this->_chunkExpected == CHUNK_DATA)
			{
				this->_body.push_back(line[i]);
				this->_received++;
				if (this->_received == this->_length)
					this->_chunkExpected = CHUNK_END_R;
			}
		}
	}
}

std::string Body::getBody() const
{
	return (this->_body);
}

bool Body::isFinished() const
{
	return (this->_state == FINISHED);
}

void Body::clear()
{
	this->_body.clear();
	this->_length = 0;
	this->_received = 0;
	this->_state = NO_BODY;
}

void Body::setChunked()
{
	this->_state = CHUNKED;
}

void Body::setContentLength(unsigned int length)
{
	this->_length = length;
	this->_state = CONTENT_LENGTH;
}