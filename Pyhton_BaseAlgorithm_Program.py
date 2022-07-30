############################# Main code ##############################
# Python program for Base Algorithm Without Paralleization 
import os
import time as time

start=time.perf_counter()

def Search(text,pattern):
  M = len(pattern)
  N = len(text)
  #print(pattern)
  #print(text)
  # build lps[] to hold the longest prefix and suffix.
  # patterns values
  lps = [0]*M
  j = 0 # index for pattern[]

  # calculate lps[] array
  computeLPSArray(pattern, M, lps)

  i = 0 # index for text[]

  while i < N:
    if pattern[j] == text[i]:
      i += 1
      j += 1

    if j == M:
      print ("Found pattern at index", str(i-j))
      j = lps[j-1]

    # mismatch after j matches
    elif i < N and pattern[j] != text[i]:
    #Avoid matching the characters lps[0..lps[j-1]], as they will match anyway.
      if j != 0:
        j = lps[j-1]
      else:
        i += 1

def computeLPSArray(pattern, m, lps):
  len = 0  # previous longest prefix-suffix length

  lps[0]  # lps[0] is always 0
  i = 1

  # lps[i] is calculated in the loop for i = 1 to m-1.
  while i < m:
    if pattern[i]== pattern[len]:
      len += 1
      lps[i] = len
      i += 1
    else:
      if len != 0:
        len = lps[len-1]

        # Note that we did not increment i in this situation.
      else:
        lps[i] = 0
        i += 1

################ Code to retrieve text and pattern files ##################
        
print(os.getcwd())
path = os.getcwd()

inputfile_path = os.path.join(path, "Python_Master.txt")

with open(inputfile_path) as masterFile:
    number_of_loop = 0
    for line in masterFile:
        # reading each word
        character = line.split()
        # Now open respective text file and pattern file and capture their content
        text_file_name = "textfile" + character[0]
        pattern_file_name = "pattern" + character[1]
        pattern_file_path = os.path.join(path, "pattern", pattern_file_name + "." + 'txt')
        text_file_path = os.path.join(path, "textfile", text_file_name + "." + 'txt')

        #print("Round is: ",number_of_loop)
        with open(pattern_file_path) as patternFile:
            for line in patternFile:
                pattern = line
        with open(text_file_path) as textFile:
            for line in textFile:
                text = line
        number_of_loop = number_of_loop + 1
        Search(text,pattern)
      
finish=time.perf_counter()
print(f'Finished in {round(finish-start,5)} second(s)')
