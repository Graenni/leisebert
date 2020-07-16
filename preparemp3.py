import os
import eyed3

colors = ['white', 'red', 'transparent', 'yellow', 'black', 'green', 'grey', 'blue']

with open('contents.txt', 'w') as outfile:
    outfile.write('L E I S E B E R T   C O N T E N T S\n')
    outfile.write('-----------------------------------\n')
    for folder in range(1, 9):
        fld = f'{folder:02d}'
        path = "./" + fld + '/'
        if not os.path.isdir(path):
            os.mkdir(path)
        files = os.listdir(path)
        tracks = []
        album = ''
        for fi, file in enumerate(files):
            filename = path + file
            if file.endswith(".mp3") or file.endswith(".MP3"):
                print(file)
                audiofile = eyed3.load(filename)
                if fi == 0:
                    album = audiofile.tag.album
                track = f'{audiofile.tag.track_num[0]:03d}'
                tracks.append((audiofile.tag.track_num[0], audiofile.tag.title))
                
                newfile = path + track + '.mp3'
                os.rename(filename, newfile)
            else:
                os.remove(filename)

        # write contents file
        tracks.sort(key=lambda x: x[0])
        outfile.write('FOLDER ' + fld + ' (' + colors[folder-1] + '): ' + album + '\n')                   
        for tr in tracks:
            outfile.write('     ' + f'{tr[0]:03d}' + ': ' + tr[1] + '\n')
        outfile.write('-----------------------------------\n')
