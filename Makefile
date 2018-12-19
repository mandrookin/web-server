all:
	g++  *.cpp -o http_serv
	echo "export HTTP_ROOT=`pwd`/webpages\n./http_serv" > start 
	chmod +x start

clean:
	rm -f http_serv start

