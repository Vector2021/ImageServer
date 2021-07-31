mysql:mysql.c
	gcc $^ -o $@ -L/usr/lib64/mysql -lmysqlclient
