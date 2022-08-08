# TestsWriter
Test writing software for teachers

## Output file format: .json
## JSON parameters:
| Name                      | Type            | Description                                                                                   | Examples                                                            |
|:--------------------------|:----------------|:----------------------------------------------------------------------------------------------|:--------------------------------------------------------------------|
| count                     | string          | Count of questions in the test                                                                | `"count": 3`                                                        |
| questions                 | array[question] | Array of data about the questions in the test                                                 | `"questions": [ {...}, {...}, {...} ]`                              |
| questions:text            | string          | Question text                                                                                 | `"text": "What is the answer?"`                                     |
| questions:question type   | string          | Question type: "with images" or "text only"                                                   | `"question type": "text only"`,<br>`"question type": "with images"` |
| questions:images          | array[string]   | Absolute paths to the files attached to the question                                          | `"images": [ "...", "...", "..." ]`,<br>`"images": []`              |
| questions:answer type     | string          | The way to answer the question: <br>"single choice",<br>"multiple choice"<br>or "text answer" | `"answer type": "single choice"`                                    |
| questions:answers         | array[string]   | Answer options to the question                                                                | `"answers": [ "...", "...", "..." ]`                                |
| questions:correct answers | array[int]      | Correct answer indices in the answer array                                                    | `"correct answers": [ 0, 3 ]`                                       |

## Sample output:
```json
{
    "count": 3,
    "questions": [
        {
            "answer type": "single choice",
            "answers": [
                "This is the answer",
                "I don't know, what is this"
            ],
            "correct answers": [
                0
            ],
            "images": [],
            "question type": "text only",
            "text": "What is the answer?"
        },
        {
            "answer type": "multiple choice",
            "answers": [
                "Answer 0",
                "Answer 1",
                "Answer 2",
                "Answer 3"
            ],
            "correct answers": [
                0,
                3
            ],
            "images": [],
            "question type": "text only",
            "text": "What is the answer?"
        },
        {
            "answer type": "text answer",
            "answers": [
                "Correct answer",
                "Also correct answer"
            ],
            "correct answers": [
                0,
                1
            ],
            "images": [
                "C:/Users/Inspirate/Pictures/unnamed.png",
                "C:/Users/Inspirate/Pictures/unnamed2.png",
                "C:/Users/Inspirate/Pictures/unnamed3.png"
            ],
            "question type": "with images",
            "text": "What is the answer?"
        }
    ]
}
```
