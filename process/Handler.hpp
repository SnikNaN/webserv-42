#pragma once

#include <string>
#include <fstream>
#include <cstring>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <ctime>

#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <exception>
#include <sys/wait.h>

#include <cstdlib>

#include <sys/types.h>
#include <csignal>

#include "../utils/utils.hpp"

#define BUFF_SIZE 1024

namespace ft
{
    class IDone
    {
    public:
        virtual bool IsDone() const = 0;
		virtual ~IDone(){};
    };

    class IInputHandler: public IDone
    {
    public:
        /* save */
        virtual void ProcessInput() = 0;
		virtual ~IInputHandler(){};
    };

    class IOutputHandler: public IDone
    {
    public:
        /* send some data from file or cgi */
        virtual void ProcessOutput() = 0;
		virtual ~IOutputHandler(){};
    };

    class OutputChunkedHandler: public IOutputHandler
    {
    private:
        int m_fd;
        std::string m_filename;
        char m_buf[BUFF_SIZE];
        bool m_isDone;
        std::ifstream   m_file;
        std::stringstream m_ss;
        OutputChunkedHandler(){};
        
    public:
        OutputChunkedHandler(int fd, const std::string& filename, const std::string& header);

        virtual bool IsDone() const;

        virtual ~OutputChunkedHandler();

        virtual void ProcessOutput();
    };

	class OutputRawHandler: public IOutputHandler
	{
	private:
		int m_fd;
		bool m_isDone;
		std::string m_text;
		std::stringstream m_ss;
		OutputRawHandler(){};

	public:
		OutputRawHandler(int fd, const std::string& text);

		virtual bool IsDone() const;

		virtual ~OutputRawHandler();

		virtual void ProcessOutput();
	};

	class InputLengthHandler: public IInputHandler
	{
	public:
		InputLengthHandler(int fd, size_t length, const std::string& remain);

		virtual bool IsDone() const;

		virtual ~InputLengthHandler() {};

		virtual void ProcessInput();

        std::string GetRes();

	private:
		int     m_fd;
		size_t	m_lengthLeft;
		bool	m_isDone;
		size_t	m_counter;
		std::stringstream m_body;

		std::string m_str; //debug

		InputLengthHandler(){};
	};

	class InputChunkedHandler: public IInputHandler
	{
	private:
		int	    m_fd;
		bool	m_isDone;
		size_t 	m_num;
		std::stringstream m_body;

		std::string search_chunk;
		bool finish;

		InputChunkedHandler(){};

	public:
		InputChunkedHandler(int fd);

		virtual bool IsDone() const;
		virtual ~InputChunkedHandler() {};
		virtual void ProcessInput();

		std::string GetRes() {return m_body.str();}
	};

    class InputCgiPostHandler: public IInputHandler
    {
    private:
        pid_t   m_pid;
        time_t  m_timer;
        int     m_pipe_to_cgi[2];
        int     m_pipe_from_cgi[2];
        std::string m_cgi_response;
        bool    m_isDone;
		bool	m_forkIsDone;
        std::stringstream m_ss;
		char m_buf[BUFF_SIZE];
		std::string m_str;
        bool    m_pipe_to_isClosed;
        bool    m_pipe_from_isClosed;

    public:
        InputCgiPostHandler();
        //InputCgiPostHandler(char** envp, char** argv, const std::string& query, int fd);

        void Init(char** envp, char** argv, const std::string& query, int fd);

        virtual bool IsDone() const;

        virtual void ProcessInput();

		std::string GetRes() {return m_ss.str();}
    };

	class InputCgiGetHandler: public IInputHandler
	{
	private:
		pid_t   m_pid;
		time_t  m_timer;
		int     m_pipe_from_cgi[2];
		std::string m_cgi_response;
		bool    m_isDone;
		bool	m_forkIsDone;
		std::stringstream m_ss;
		char m_buf[BUFF_SIZE];

	public:
		InputCgiGetHandler();

        void Init(char** envp, char** argv, int fd);

		virtual bool IsDone() const;

		virtual void ProcessInput();

		std::string GetRes() {return m_ss.str();}
	};
}   //namespace ft
