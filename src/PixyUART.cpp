#include "PixyUART.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>

namespace Pixy {

PixyUART::PixyUART( const std::string& device )
  : m_device(device), m_fd(-1)
{
    ;
}

PixyUART::~PixyUART()
{
    close();
}

bool PixyUART::open()
{
    if (isOpen()) close();
#ifdef TRACE
    std::cerr << "PixyUART::open" << std::endl;
#endif
    m_fd = ::open(m_device.c_str(), O_RDWR);
    if (m_fd == -1) {
    	perror("open");
    	return false;
    }

    struct termios t;
    std::memset(&t, 0, sizeof t);
    t.c_cflag = CS8 | CREAD | CLOCAL;
    cfsetspeed(&t, B115200);
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;

    if (tcsetattr(m_fd, TCSANOW, &t) == -1) {
	perror("tcsetattr");
	close();
	return false;
    }


#ifdef TRACE
    std::cerr << "PixyUART::open OK, fd = " << m_fd << std::endl;
#endif
    return true;
}

bool PixyUART::close()
{
#ifdef TRACE
	std::cerr << "PixyUART::close" << std::endl;
#endif
    if (isOpen()) {
	::close(m_fd);
	m_fd = -1;
    }
    return true;
}

bool PixyUART::isOpen() const
{
    return (m_fd != -1);
}

int PixyUART::getByte()
{
    uint8_t b;
    int n;
    if (isOpen()) {
	do {
	    n = read(m_fd, &b, 1);
	    if (n == -1) {
		perror("PixyUART::getByte");
		return -1;
	    }
	} while (n != 1);
    	char buf[3];
    	snprintf(buf, 3, "%02x", b);
#ifdef TRACE
    	std::cerr << " " << buf;
#endif
	return b;
    } else {
	return -1;
    }
}

int PixyUART::getWord()
{
    int low, high;
    if ((low = getByte()) == -1) {
	return -1;
    }
    if ((high = getByte()) == -1) {
	return -1;
    }
    return (high << 8) | low;
}

int PixyUART::putString( const std::string& str )
{
    if (isOpen())
	return write(m_fd, str.c_str(), str.size());
    else
	return -1;
}

}; // namespace Pixy

