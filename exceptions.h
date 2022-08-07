#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <string>
#include <exception>


class BaseException : public std::exception
{
public:
    explicit BaseException(std::string msg = "Не удалось выполнить действие.") : _msg(msg) { }

    virtual const char *what() const noexcept override { return _msg.c_str(); }

protected:
    std::string _msg;
};

class TestsWriterException : public BaseException
{
public:
    explicit TestsWriterException(std::string msg = "Не удалось выполнить действие.") : BaseException(msg) { }
};

class SavingException : public TestsWriterException
{
public:
    explicit SavingException(std::string msg = "При сохранении теста возникла ошибка.") : TestsWriterException(msg) { }
};

#endif // EXCEPTIONS_H
