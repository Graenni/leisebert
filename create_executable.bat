py -m venv ./pivenv/
.\pivenv\Scripts\activate.bat & ^
pip install -r requirements.txt & ^
pyinstaller -F -i "lb.ico" preparemp3.py & ^
deactivate & ^
rmdir /q /s .\pivenv\ & ^
rmdir /q /s .\build\ & ^
rmdir /q /s .\__pycache__ & ^
del preparemp3.spec
