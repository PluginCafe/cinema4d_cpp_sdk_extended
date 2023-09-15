import hashlib
import sys

def calculateMD5(fileName):
    print(fileName)
    return hashlib.md5(open(fileName,'rb').read()).hexdigest()

if __name__ == "__main__":
    arg = sys.argv[1];
    hash = calculateMD5(arg)
    print(hash)

