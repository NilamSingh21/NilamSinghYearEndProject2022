######## Main code ###############
# Python program with Paralleization

import os
import concurrent.futures
import multiprocessing as mp
import time as time
from multiprocessing import Process, Manager

# Step 1: Init multiprocessing.Pool()
pool = mp.Pool(mp.cpu_count())
print ('Number of cores',mp.cpu_count())

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
  len = 0 # previous longest prefix-suffix length

  lps[0] # lps[0] is always 0
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

        # Also, note that we do not increment i here
      else:
        lps[i] = 0
        i += 1

################ Code to retrieve text and pattern #######
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
data=[]
# Step 2: `pool.apply` the `Search()`
results = [pool.apply(Search, args=(text,pattern)) for row in data]
# Step 3: Don't forget to close
pool.close() 

'''
#We can do Paralleization using below method also
#method 2: Paralleization

 with concurrent.futures.ProcessPoolExecutor() as executer:
results=executer.map(Search,text,pattern) 
 ''' 

finish=time.perf_counter()
print(f'Finished in {round(finish-start,5)} second(s)')






