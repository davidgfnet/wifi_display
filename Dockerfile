FROM php:7.3-apache
RUN echo "deb http://http.debian.net/debian/ buster main contrib non-free" > /etc/apt/sources.list && \
    echo "deb http://http.debian.net/debian/ buster-updates main contrib non-free" >> /etc/apt/sources.list && \
    echo "deb http://security.debian.org/ buster/updates main contrib non-free" >> /etc/apt/sources.list && \
    apt-get update
RUN apt-get install -y --no-install-recommends \
    imagemagick \
    ghostscript \
    libmagickwand-dev \
    librsvg2-bin \
    xfonts-100dpi \
    xfonts-75dpi \
    xfonts-base \
    fonts-roboto \
    fonts-inconsolata \
    ttf-mscorefonts-installer \
    fonts-open-sans \
    fontconfig 
COPY server/fontconfig/* /etc/fonts/conf.d/
RUN fc-cache -f -v
RUN printf "\n" | pecl install imagick
RUN mkdir -p /tmp
RUN chmod a+rw /tmp

RUN docker-php-ext-enable imagick