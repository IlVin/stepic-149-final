echo 'https://github.com/IlVin/stepic-149-final.git' > ~/final.txt

mkdir ~/libev
git clone https://github.com/enki/libev.git ~/libev
cd ~/libev && ./configure
cd ~/libev && make
cd ~/libev && make install
