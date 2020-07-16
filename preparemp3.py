import os
import eyed3

for folder in range(1, 9):
    path = "./" + str(folder) + '/'
    if not os.path.isdir(path):
        os.mkdir(path)

    files = os.listdir(path)
    for file in files:
        filename = path + file
        if file.endswith(".mp3") or file.endswith(".MP3"):
            print(file)
            audiofile = eyed3.load(filename)
            newfile = path + f'{audiofile.tag.track_num[0]:03d}' + '.mp3'
            os.rename(filename, newfile)
        else:
            os.remove(filename)
