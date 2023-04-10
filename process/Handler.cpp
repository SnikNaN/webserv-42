#include "Handler.hpp"

namespace ft
{
    OutputChunkedHandler::OutputChunkedHandler(int fd, const std::string& filename, const std::string& header) :
            m_fd(fd),
            m_filename(filename),
            m_isDone(false)
    {
        m_ss << header;

        m_file.open(m_filename.c_str(), std::ios::in);

        if (!m_file.is_open())
        {
            switch (errno)
            {
                case ENOENT:
                    throw NotFound;
                case EPERM:
                case EACCES:
                    throw Forbidden;
                case 36:
                    throw ReqUriTooLong;
                default:
                    throw IntServerErr;
            }
        }

    }

    bool OutputChunkedHandler::IsDone() const
    {
        return m_isDone;
    }

    OutputChunkedHandler::~OutputChunkedHandler()
    {
        m_file.close();
    }

    void OutputChunkedHandler::ProcessOutput()
    {
        if (!m_file.eof() || !m_isDone)
        {
            if (m_ss.str().empty())
            {
                m_file.read(m_buf, BUFF_SIZE);
                std::streamsize size = m_file.gcount();
                m_ss << std::hex << size;
                m_ss << "\r\n";
                m_ss.write(m_buf, size);
                m_ss << "\r\n";
                if (m_file.eof())
                {
                    m_ss << "0\r\n\r\n";
                }
            }
            ssize_t cnt = send(m_fd, m_ss.str().c_str(), m_ss.str().size(), 0);

            if (cnt <= 0)
                throw std::runtime_error("OutputChunkedHandler send error");

            if ((size_t)cnt < m_ss.str().size())
                m_ss.str(m_ss.str().substr(cnt, m_ss.str().size() - cnt));
            else
                m_ss.str("");
            if (m_file.eof() && m_ss.str().empty())
                m_isDone = true;
        }
    }

	OutputRawHandler::OutputRawHandler(int fd, const std::string& text) :
		m_fd(fd),
		m_isDone(false),
		m_text(text)
	{
		m_ss << text;
	}

	bool OutputRawHandler::IsDone() const
	{
		return m_isDone;
	}

	OutputRawHandler::~OutputRawHandler()
	{
	}

	void OutputRawHandler::ProcessOutput()
	{
		if (!m_isDone)
		{
			ssize_t cnt = send(m_fd, m_text.c_str(),
							  m_text.size() > BUFF_SIZE ? BUFF_SIZE : m_text.size(), 0);
            if (cnt <= 0)
                throw std::runtime_error("OutputChunkedHandler send error");
			if ((size_t)cnt < m_text.size())
				m_text = m_text.substr(cnt, m_text.size() - cnt);
			else
				m_text.clear();
			if (m_text.empty())
				m_isDone = true;
		}
	}

    //-----------------------------------------------------------------------------------

    InputLengthHandler::InputLengthHandler(int fd, size_t length, const std::string& remain) :
            m_fd(fd),
            m_lengthLeft(length - remain.size()),
            m_isDone(m_lengthLeft == 0),
			m_body(remain)
    {
        std::cout << "<<<<remain" << remain.size() << std::endl;
        std::cout << "<<<<init" << length - remain.size() << std::endl;
        m_body << remain;
        m_counter = 0;
    }

    bool InputLengthHandler::IsDone() const
    {
        return m_isDone;
    }

    void InputLengthHandler::ProcessInput()
    {
            ssize_t readRes;
            char buffer[BUFF_SIZE];

            ssize_t readSize = BUFF_SIZE > m_lengthLeft ? m_lengthLeft : BUFF_SIZE;
            readRes = recv(m_fd, buffer, readSize, 0);
            if (readRes <= 0)
            {
                throw std::runtime_error("InputLengthHandler receive error");
            }

            m_counter += readRes;
            m_lengthLeft -= readRes;
            m_body.write(buffer, readRes);

            if (m_lengthLeft == 0)
                m_isDone = true;
    }

    std::string InputLengthHandler::GetRes()
    {
        return m_body.str();
    }



    // ----------------------------------------------------------------

    InputChunkedHandler::InputChunkedHandler(int fd) :
            m_fd(fd),
            m_isDone(false),
            m_num(0),
            finish(false)
    {

    }

    void InputChunkedHandler::ProcessInput()
    {
        char buf[BUFF_SIZE];
        std::string	body;
        std::string tmp;
        std::stringstream ss;
        ssize_t cnt;
        size_t pos;

        if (m_num != 0 )
        {
            std::fill_n(buf, BUFF_SIZE, '\0');
            cnt = recv(m_fd, buf, m_num < (BUFF_SIZE - 1) ? m_num : (BUFF_SIZE - 1), 0);
            if (cnt <= 0)
            {
                throw std::runtime_error("InputChunkedHandler receive error");
            }

            if (m_num - cnt >= 2)
                m_body << buf;
            else if (m_num > 2)
            {
                body = buf;
                m_body << body.substr(0, body.size() - (2 - (m_num - cnt)));
            }
            m_num -= cnt;
            if (m_num == 0 && finish)
                m_isDone = true;
        }
        else
        {
            if ((pos = search_chunk.find("\r\n")) != std::string::npos)
            {
                ss << search_chunk.substr(0, pos);
                ss >> std::hex >> m_num;
                if (m_num == 0)
                    finish = true;
                m_num += 2;
                tmp = search_chunk.substr(pos + 2, search_chunk.size() - (pos - 2));
                if (tmp.size() >= m_num)
                {
                    m_body << tmp.substr(0, m_num - 2);
                    search_chunk = tmp.substr(m_num, tmp.size() - m_num);
                    m_num = 0;
                }
                else
                {
                    m_num -= tmp.size();
                    if (m_num >= 2)
                        m_body << tmp;
                    else
                        m_body << tmp.substr(0, tmp.size() - (2 - m_num));
                    search_chunk = "";
                }
            }
            else
            {
                std::fill_n(buf, BUFF_SIZE, '\0');
                cnt = recv(m_fd, buf, 20, 0);

                search_chunk.append(buf);
            }
        }
        if (m_num == 0 && finish)
            m_isDone = true;
    }

    bool InputChunkedHandler::IsDone() const
    {
        return m_isDone;
    }

    InputCgiPostHandler::InputCgiPostHandler() :    m_isDone(false),
                                                    m_forkIsDone(false),
                                                    m_pipe_to_isClosed(false),
													m_pipe_from_isClosed(false)
    {
    }

    void InputCgiPostHandler::Init(char **envp, char **argv, const std::string &query, int fd)
    {
        m_str = query;

        if (pipe(m_pipe_to_cgi) == -1)
            throw IntServerErr;
        if (pipe(m_pipe_from_cgi) == -1)
        {
            close(m_pipe_to_cgi[0]);
            close(m_pipe_to_cgi[1]);
            throw IntServerErr;
        }
        m_pid = fork();
        if (m_pid == -1)
        {
            close(m_pipe_to_cgi[0]);
            close(m_pipe_to_cgi[1]);
            close(m_pipe_from_cgi[0]);
            close(m_pipe_from_cgi[1]);
            throw IntServerErr;
            //throw error 500
        }
        else if (m_pid == 0)
        {
            //fork
            close(fd);
            dup2(m_pipe_to_cgi[0], 0);
            close(m_pipe_to_cgi[0]);
            close(m_pipe_to_cgi[1]);
            dup2(m_pipe_from_cgi[1], 1);
            close(m_pipe_from_cgi[0]);
            close(m_pipe_from_cgi[1]);

            if (execve(argv[0], argv, envp) == -1)
            {
                //error 502;
            }
            _exit(EXIT_FAILURE);
        }
        else if (m_pid > 0) {
            //this
            std::time(&m_timer);
            close(m_pipe_to_cgi[0]);
            close(m_pipe_from_cgi[1]);
            fcntl(m_pipe_to_cgi[1], F_SETFL, O_NONBLOCK);
            fcntl(m_pipe_from_cgi[0], F_SETFL, O_NONBLOCK);
        }
    }

    void InputCgiPostHandler::ProcessInput()
    {
		int status;
        if (!m_isDone)
		{
			if (!m_forkIsDone)
			{
				if (waitpid(m_pid, &status, WNOHANG) != m_pid)
				{
					if (std::time(NULL) - m_timer > 15)
					{
						std::cout << "time out\n";
						kill(m_pid, SIGKILL); // throw error 504
						m_isDone = true;
						close(m_pipe_from_cgi[0]);
						close(m_pipe_to_cgi[1]);
                        throw GatewayTimeout;
					}
				}
				else
				{
                    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS)
                    {
                        close(m_pipe_to_cgi[1]);
                        close(m_pipe_from_cgi[0]);
                        throw BadGateway;
                    }
					m_forkIsDone = true;
				}
			}

			if (!m_str.empty())
			{
				ssize_t written = write(m_pipe_to_cgi[1], m_str.c_str(),
										m_str.size() < BUFF_SIZE ? m_str.size() : BUFF_SIZE);
				if (written > 0)
				{
					m_str = m_str.substr(written);
				}
//                else if (written < 0)
//                {
//                    if (!m_pipe_to_isClosed)
//                        close(m_pipe_to_cgi[1]);
//                    if (!m_pipe_from_isClosed)
//                        close(m_pipe_from_cgi[0]);
//                    throw IntServerErr;
//                }
			}

			ssize_t len = read(m_pipe_from_cgi[0], m_buf, BUFF_SIZE);
			if (len > 0)
				m_ss.write(m_buf, len);
//            else if (len < 0)
//            {
//                close(m_pipe_to_cgi[1]);
//                close(m_pipe_from_cgi[0]);
//                throw IntServerErr;
//            }
            if (m_str.empty() && !m_pipe_to_isClosed)
            {
                close(m_pipe_to_cgi[1]);
                m_pipe_to_isClosed = true;
            }
			if (len == 0 && m_str.empty())
			{
				close(m_pipe_from_cgi[0]);
                m_pipe_from_isClosed = true;
				m_isDone = true;
			}
        }
    }

    bool InputCgiPostHandler::IsDone() const {
        return m_isDone;
    }

    InputCgiGetHandler::InputCgiGetHandler() :  m_isDone(false)
    {}

	void InputCgiGetHandler::Init(char **envp, char **argv, int fd)
	{
		if (pipe(m_pipe_from_cgi) == -1)
            throw IntServerErr;

		m_pid = fork();
		if (m_pid == -1)
		{
            close(m_pipe_from_cgi[0]);
            close(m_pipe_from_cgi[1]);
            throw IntServerErr;
		}
		else if (m_pid == 0)
		{
			//fork
            close(fd);
			dup2(m_pipe_from_cgi[1], 1);
			close(m_pipe_from_cgi[0]);
			close(m_pipe_from_cgi[1]);

			if (execve(argv[0], argv, envp) == -1)
			{
				//error 502;
			}
			_exit(EXIT_FAILURE);
		}
		else if (m_pid > 0) {
			//this
			std::time(&m_timer);
			close(m_pipe_from_cgi[1]);
			fcntl(m_pipe_from_cgi[0], F_SETFL, O_NONBLOCK);
		}
	}

	void InputCgiGetHandler::ProcessInput()
	{
		int status;
		if (!m_isDone)
		{
			if (!m_forkIsDone)
			{
				if (waitpid(m_pid, &status, WNOHANG) != m_pid)
				{
					if (std::time(NULL) - m_timer > 15)
					{
						std::cout << "time out\n";
						kill(m_pid, SIGKILL); // throw error 504
						m_isDone = true;
						close(m_pipe_from_cgi[0]);
                        throw GatewayTimeout;
					}
				}
				else
				{
                    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS)
                    {
                        close(m_pipe_from_cgi[0]);
                        throw BadGateway;
                    }
					m_forkIsDone = true;
				}
			}

			ssize_t len = read(m_pipe_from_cgi[0], m_buf, BUFF_SIZE);
			if (len > 0)
				m_ss.write(m_buf, len);
//            if (len < 0)
//            {
//                close(m_pipe_from_cgi[0]);
//                throw IntServerErr;
//            }
			if (len == 0)
			{
				close(m_pipe_from_cgi[0]);
				m_isDone = true;
			}
		}
	}

	bool InputCgiGetHandler::IsDone() const
	{
		return m_isDone;
	}

}   //namespace ft
