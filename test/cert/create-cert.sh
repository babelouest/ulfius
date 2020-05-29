#!/bin/sh

DEST=./cert
RET=0

# clean old certs
rm -f $DEST/server.* $DEST/root* $DEST/client*

echo >> $DEST/certtool.log
echo Generate Ulfius test certificates >> $DEST/certtool.log
echo >> $DEST/certtool.log

# www cert
certtool --generate-privkey --outfile $DEST/server.key --sec-param High 2>>$DEST/certtool.log
STATUS=$?
if [ $STATUS -eq 0 ]; then
  printf "server.key         \033[0;32mOK\033[0m\n"
else
  printf "server.key         \033[0;31mError\033[0m\n"
  RET=$STATUS
fi
certtool --generate-self-signed --load-privkey $DEST/server.key --outfile $DEST/server.crt --template $DEST/template-server.cfg 2>>$DEST/certtool.log
STATUS=$?
if [ $STATUS -eq 0 ]; then
  printf "server.crt         \033[0;32mOK\033[0m\n"
else
  printf "server.crt         \033[0;31mError\033[0m\n"
  RET=$STATUS
fi

# CA root
certtool --generate-privkey --outfile $DEST/root1.key --sec-param High 2>>$DEST/certtool.log
STATUS=$?
if [ $STATUS -eq 0 ]; then
  printf "root1.key          \033[0;32mOK\033[0m\n"
else
  printf "root1.key          \033[0;31mError\033[0m\n"
  RET=$STATUS
fi
certtool --generate-self-signed --load-privkey $DEST/root1.key --outfile $DEST/root1.crt --template $DEST/template-ca.cfg 2>>$DEST/certtool.log
STATUS=$?
if [ $STATUS -eq 0 ]; then
  printf "root1.crt          \033[0;32mOK\033[0m\n"
else
  printf "root1.crt          \033[0;31mError\033[0m\n"
  RET=$STATUS
fi

# client 1
certtool --generate-privkey --outfile $DEST/client1.key --sec-param High 2>>$DEST/certtool.log
STATUS=$?
if [ $STATUS -eq 0 ]; then
  printf "client1.key        \033[0;32mOK\033[0m\n"
else
  printf "client1.key        \033[0;31mError\033[0m\n"
  RET=$STATUS
fi
certtool --generate-certificate --load-privkey $DEST/client1.key --load-ca-certificate $DEST/root1.crt --load-ca-privkey $DEST/root1.key --outfile $DEST/client1.crt --template $DEST/template-client.cfg 2>>$DEST/certtool.log
STATUS=$?
if [ $STATUS -eq 0 ]; then
  printf "client1.crt        \033[0;32mOK\033[0m\n"
else
  printf "client1.crt        \033[0;31mError\033[0m\n"
  RET=$STATUS
fi

# CA root 2
certtool --generate-privkey --outfile $DEST/root2.key --sec-param High 2>>$DEST/certtool.log
STATUS=$?
if [ $STATUS -eq 0 ]; then
  printf "root2.key          \033[0;32mOK\033[0m\n"
else
  printf "root2.key          \033[0;31mError\033[0m\n"
  RET=$STATUS
fi
certtool --generate-self-signed --load-privkey $DEST/root2.key --outfile $DEST/root2.crt --template $DEST/template-ca2.cfg 2>>$DEST/certtool.log
STATUS=$?
if [ $STATUS -eq 0 ]; then
  printf "root2.crt          \033[0;32mOK\033[0m\n"
else
  printf "root2.crt          \033[0;31mError\033[0m\n"
  RET=$STATUS
fi

# client 2
certtool --generate-privkey --outfile $DEST/client2.key --sec-param High 2>>$DEST/certtool.log
STATUS=$?
if [ $STATUS -eq 0 ]; then
  printf "client2.key        \033[0;32mOK\033[0m\n"
else
  printf "client2.key        \033[0;31mError\033[0m\n"
  RET=$STATUS
fi
certtool --generate-certificate --load-privkey $DEST/client2.key --load-ca-certificate $DEST/root2.crt --load-ca-privkey $DEST/root2.key --outfile $DEST/client2.crt --template $DEST/template-client.cfg 2>>$DEST/certtool.log
STATUS=$?
if [ $STATUS -eq 0 ]; then
  printf "client2.crt        \033[0;32mOK\033[0m\n"
else
  printf "client2.crt        \033[0;31mError\033[0m\n"
  RET=$STATUS
fi

# client 3 self-signed
certtool --generate-privkey --outfile $DEST/client3.key --sec-param High 2>>$DEST/certtool.log
STATUS=$?
if [ $STATUS -eq 0 ]; then
  printf "client3.key        \033[0;32mOK\033[0m\n"
else
  printf "client3.key        \033[0;31mError\033[0m\n"
  RET=$STATUS
fi
certtool --generate-self-signed --load-privkey $DEST/client3.key --outfile $DEST/client3.crt --template $DEST/template-client.cfg 2>>$DEST/certtool.log
STATUS=$?
if [ $STATUS -eq 0 ]; then
  printf "client3.crt        \033[0;32mOK\033[0m\n"
else
  printf "client3.crt        \033[0;31mError\033[0m\n"
  RET=$STATUS
fi
