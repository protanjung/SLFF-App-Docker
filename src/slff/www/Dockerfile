FROM php:7.2-apache

ADD https://github.com/mlocati/docker-php-extension-installer/releases/latest/download/install-php-extensions /usr/local/bin/

RUN chmod +x /usr/local/bin/install-php-extensions && \
    sync && \ 
    install-php-extensions gd xdebug zip

COPY config.php /var/www/html/config.php
COPY index.php /var/www/html/index.php
COPY translation.json /var/www/html/translation.json

RUN /bin/bash -c 'mkdir -p /var/www/html/slff-data'