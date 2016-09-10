#include <obs-module.h>

struct rtmp_custom {
	char *server, *key;
	bool use_auth;
	char *username, *password;
};

static const char *rtmp_custom_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("CustomStreamingServer");
}

static void rtmp_custom_update(void *data, obs_data_t *settings)
{
	struct rtmp_custom *service = data;

	bfree(service->server);
	bfree(service->key);

	const char *url = obs_data_get_string(settings, "upload_url");

	const char *p = NULL, *pserver = NULL, *pkey = NULL;
	char  server[1024] = {0};
	int   found = 0, n = 0;

	bool url_error = false;

	if (url == NULL || strlen(url) < sizeof("rtmp://")) {
		url_error = true;
	}

	// trim left
	while (url && *url == ' ') {
		url++;
	}

	if (strlen(url) < sizeof("rtmp://")) {
		url_error = true;
	}

	p = url;

	// trim right.
	{
		p = p + strlen(p) - 1;
		while (*p == ' ') {
			*((char*)p) = '\0';
			p--;
		}
	}

	if (strlen(url) < sizeof("rtmp://") || strstr(url, "rtmp://") != url) {
		url_error = true;
	}

	// find the key and server
	p = url + sizeof("rtmp://");
	while (p && *p != '\0') {
		if (*p++ == '/') {
			n++;
			if (n >= 2) {
				found = 1;
				break;
			}
		}
	}

	if (found) {
		pserver = url;
		pkey = p;

		memcpy(server, pserver, pkey - url - 1);
		service->server = bstrdup(server);
		service->key = bstrdup(pkey);

		// reset back the string to edit-box here.
		obs_data_set_string(settings, "upload_url", url);
	} else {
		url_error = true;
	}

	if ( url_error )
	{
		service->server = bstrdup("");
		service->key = bstrdup("");
	}

// 	service->server = bstrdup(obs_data_get_string(settings, "server"));
// 	service->key    = bstrdup(obs_data_get_string(settings, "key"));
	service->use_auth = obs_data_get_bool(settings, "use_auth");
	service->username = bstrdup(obs_data_get_string(settings, "username"));
	service->password = bstrdup(obs_data_get_string(settings, "password"));
}

static void rtmp_custom_destroy(void *data)
{
	struct rtmp_custom *service = data;

	bfree(service->server);
	bfree(service->key);
	bfree(service->username);
	bfree(service->password);
	bfree(service);
}

static void *rtmp_custom_create(obs_data_t *settings, obs_service_t *service)
{
	struct rtmp_custom *data = bzalloc(sizeof(struct rtmp_custom));
	rtmp_custom_update(data, settings);

	UNUSED_PARAMETER(service);
	return data;
}

static bool use_auth_modified(obs_properties_t *ppts, obs_property_t *p,
	obs_data_t *settings)
{
	bool use_auth = obs_data_get_bool(settings, "use_auth");
	p = obs_properties_get(ppts, "username");
	obs_property_set_visible(p, use_auth);
	p = obs_properties_get(ppts, "password");
	obs_property_set_visible(p, use_auth);
	return true;
}

static obs_properties_t *rtmp_custom_properties(void *unused)
{
	UNUSED_PARAMETER(unused);

	obs_properties_t *ppts = obs_properties_create();
	obs_property_t *p;

	obs_properties_add_text(ppts, "upload_url", obs_module_text("UploadURL"), OBS_TEXT_DEFAULT);
// 	obs_properties_add_text(ppts, "server", "URL", OBS_TEXT_DEFAULT);
// 	obs_properties_add_text(ppts, "key", obs_module_text("StreamKey"), OBS_TEXT_PASSWORD);

	p = obs_properties_add_bool(ppts, "use_auth", obs_module_text("UseAuth"));
	obs_properties_add_text(ppts, "username", obs_module_text("Username"),
			OBS_TEXT_DEFAULT);
	obs_properties_add_text(ppts, "password", obs_module_text("Password"),
			OBS_TEXT_PASSWORD);
	obs_property_set_modified_callback(p, use_auth_modified);
	return ppts;
}

static const char *rtmp_custom_url(void *data)
{
	struct rtmp_custom *service = data;
	return service->server;
}

static const char *rtmp_custom_key(void *data)
{
	struct rtmp_custom *service = data;
	return service->key;
}

static const char *rtmp_custom_username(void *data)
{
	struct rtmp_custom *service = data;
	if (!service->use_auth)
		return NULL;
	return service->username;
}

static const char *rtmp_custom_password(void *data)
{
	struct rtmp_custom *service = data;
	if (!service->use_auth)
		return NULL;
	return service->password;
}

struct obs_service_info rtmp_custom_service = {
	.id             = "rtmp_custom",
	.get_name       = rtmp_custom_name,
	.create         = rtmp_custom_create,
	.destroy        = rtmp_custom_destroy,
	.update         = rtmp_custom_update,
	.get_properties = rtmp_custom_properties,
	.get_url        = rtmp_custom_url,
	.get_key        = rtmp_custom_key,
	.get_username   = rtmp_custom_username,
	.get_password   = rtmp_custom_password
};
