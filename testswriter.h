#ifndef TESTSWRITER_H
#define TESTSWRITER_H

#include <QString>
#include <QVector>
#include <fstream>



class TestsWriter
{
public:

    class Question
    {
    public:
        enum AnswerType
        {
            NONE,
            SINGLE_CHOICE,
            MULTIPLE_CHOICE,
            STRING_ANSWER
        };

        Question();
        Question(const QString &question,
                 const QVector<QString> &filenames,
                 const AnswerType &answerType,
                 const QVector<QString> &answers,
                 const QVector<size_t> &correctAnswerIndicies);

        QString getQuestionText() const;
        QVector<QString> getFilenames() const;
        AnswerType getAnswerType() const;
        QVector<QString> getAnswers() const;
        QVector<size_t> getCorrectAnswerIndicies() const;
        void setReadable(bool flag);
        bool readable() const;

    private:
        QString _question;
        QVector<QString> _filenames;
        AnswerType _answerType;
        QVector<QString> _answers;
        QVector<size_t> _correctAnswerIndicies;
        bool _isReadable;
    };

    void setQuestionsCount(size_t count);
    bool addQuestion(const Question &question, size_t index);
    bool getQuestion(Question &question, size_t index) const;
    QVector<size_t> getIncompleteQuestionIndicies() const;
    void saveTest(std::ofstream &file);
    void resetTest();

private:
    size_t _questionsCount = 0;
    QVector<Question> _questions;
};

#endif // TESTSWRITER_H
