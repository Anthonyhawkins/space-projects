FROM httpd:2.4

COPY ephemerides.conf /usr/local/apache2/conf/extra/ephemerides.conf

RUN printf '\nInclude conf/extra/ephemerides.conf\n' >> /usr/local/apache2/conf/httpd.conf

EXPOSE 80
