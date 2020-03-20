import sys
name = sys.argv[1]
f1 = open(name, "rb")
f2 = open("data/"+name, "rb")
content1 = f1.read()
content2 = f2.read()
print(content1 == content2)