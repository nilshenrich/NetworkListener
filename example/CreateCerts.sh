#!/bin/bash

# Create a self-signed certificate for the localhost
# Structure:
#   keys/
#       ca/
#           ca_key.pem
#           ca_cert.pem
#       server/
#           server_key.pem
#           server_cert.pem
#       client/
#           client_key.pem
#           client_cert.pem

# Get localization
C=$(curl -s http://ipinfo.io/json | grep -Po '"country":\s*"\K[^"]*')
ST=$(curl -s http://ipinfo.io/json | grep -Po '"region":\s*"\K[^"]*')
L=$(curl -s http://ipinfo.io/json | grep -Po '"city":\s*"\K[^"]*')

# Remove all old cert files
rm -r keys/

# Create folder structure
mkdir keys/
mkdir keys/ca/
mkdir keys/server/
mkdir keys/client/

# Create self-signed CA certificate and key
openssl req -x509 -newkey rsa:4096 -days 365 -nodes -sha512 -utf8 \
    -keyout keys/ca/ca_key.pem \
    -out keys/ca/ca_cert.pem \
    -subj "/C=${C}/ST=${ST}/L=${L}/O=Networking/OU=Networking.Authority/CN=localhost"

# Create server private key and certificate signing request and sign it with the CA
openssl req -newkey rsa:4096 -nodes -sha512 -utf8 \
    -keyout keys/server/server_key.pem \
    -out keys/server/server_req.csr \
    -subj "/C=${C}/ST=${ST}/L=${L}/O=Networking/OU=Networking.Server/CN=localhost"
openssl x509 -req -days 365 \
    -in keys/server/server_req.csr \
    -CA keys/ca/ca_cert.pem \
    -CAkey keys/ca/ca_key.pem \
    -CAcreateserial \
    -out keys/server/server_cert.pem

# Create client private key and certificate signing request and sign it with the CA
openssl req -newkey rsa:4096 -nodes -sha512 -utf8 \
    -keyout keys/client/client_key.pem \
    -out keys/client/client_req.csr \
    -subj "/C=${C}/ST=${ST}/L=${L}/O=Networking/OU=Networking.Client/CN=localhost"
openssl x509 -req -days 365 \
    -in keys/client/client_req.csr \
    -CA keys/ca/ca_cert.pem \
    -CAkey keys/ca/ca_key.pem \
    -CAcreateserial \
    -out keys/client/client_cert.pem
