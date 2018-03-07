from random import randint
import string

def text_wirter():
    file = open("test.txt", "w")
    letters = string.ascii_letters
    letters += " "

    for x in range(1, 10000):
        s = str(x)
        for i in range(1023 - len(s)):
            s += letters[randint(0, len(letters)-1)]
        s += "\n"
        file.write(s)

text_wirter()