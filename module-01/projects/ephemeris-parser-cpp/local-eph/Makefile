IMAGE_NAME := local-eph-httpd
CONTAINER_NAME := local-eph-httpd
CACHE_DIR := $(CURDIR)/cache
CONTAINER_CACHE_DIR := /usr/local/apache2/htdocs/public-files/ephemerides

.PHONY: local-up local-down local-logs

local-up:
	docker build -t $(IMAGE_NAME) .
	-docker rm -f $(CONTAINER_NAME)
	docker run -d --rm \
		--name $(CONTAINER_NAME) \
		-p 80:80 \
		-v "$(CACHE_DIR):$(CONTAINER_CACHE_DIR):ro" \
		$(IMAGE_NAME)

local-down:
	-docker rm -f $(CONTAINER_NAME)

local-logs:
	docker logs $(CONTAINER_NAME)
