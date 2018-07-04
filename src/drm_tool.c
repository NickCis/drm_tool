#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <inttypes.h>

#include <dirent.h>
#include <sys/types.h>

#define DRM_PATH "/dev/dri/"

typedef enum {
	DRM_TOOL_LIST=0,
	DRM_TOOL_SET
} DrmToolCmd;

typedef struct {
	DrmToolCmd cmd;
	const char* card;
	int connector_id;
	int value;
	const char* prop_name;
	int drm_fd;
} DrmToolArgs;

int usage(const char* name)
{
	printf("%s <event> \n", name);
	printf("  event: list or set.\n");
	printf("    * list: Prints all drm cards with information\n");
	printf("    * set: in order to set the value of a property, \n");
	printf("      %s set <card> <connector id> <property> <value>\n", name);
	printf("        card: card file '/dev/dri/'\n");
	printf("        connector id: the id of the connector (use 'list' in order to get this info)\n");
	printf("        property: property name\n");
	printf("        value: \n");
	printf("\n");
	printf("Example:\n");
	printf("  %s set /dev/dri/card0 19 audio 2\n", name);
	printf("    Note: In order to enable audio output, Xorg uses the value '2', i really dont know why\n");
	return 0;
}

const char* get_card_name(DrmToolArgs* args)
{
	static DIR *dir = NULL;
	static char card_path[255];
	static struct dirent *ent;

	if(args->cmd == DRM_TOOL_SET)
		return args->card;

	if(dir == NULL){
		if( (dir = opendir(DRM_PATH)) == NULL){
			printf("Cannot open folder '%s', No drm cards found\n", DRM_PATH);
			return NULL;
		}
	}

	while((ent = readdir(dir)) != NULL){
		char* file = ent->d_name;
		if(file[0] != 'c' || file[1] != 'a' || file[2] != 'r' || file[3] != 'd')
			continue;

		sprintf(card_path, "%s%s", DRM_PATH, file);
		return card_path;
	}

	closedir(dir);
	return NULL;
}

void drm_set_property(DrmToolArgs* args,  drmModeConnector *conn, drmModePropertyPtr prop, uint64_t old_value)
{
	switch(args->cmd){
		case DRM_TOOL_SET:
			{
				if(strcmp(prop->name, args->prop_name))
					return;

				int ret =  drmModeConnectorSetProperty(args->drm_fd, conn->connector_id, prop->prop_id, args->value);
				printf("Setting '%s' -> %d - ret: %d :: connector_id: %d prop_id: %d \n", args->prop_name, args->value, ret, conn->connector_id, prop->prop_id);
			}
			break;

		case DRM_TOOL_LIST:
		default:
		{
			char* enum_name = NULL;
			// If enum => find enum text
			if (prop->flags & DRM_MODE_PROP_ENUM) {
				for (int i = 0; i < prop->count_enums; i++) {
					if (prop->enums[i].value == old_value) {
						enum_name = prop->enums[i].name;
					}
				}
				if (!enum_name) {
					printf("#\tproperty value (%lu) not found in enum list!\n", old_value);
				}
			}
			printf("    property (#%d):  %s = %lu", prop->prop_id, prop->name, old_value);
			if (enum_name)
				printf(" (%s)", enum_name);
			printf("\n");
			break;
		}
	}
}



void print_drm_connector(DrmToolArgs* args, drmModeConnector *conn)
{
	if(args->cmd != DRM_TOOL_LIST)
		return;

	printf("  Connector: %d\n", conn->connector_id);
	if (conn->connection == DRM_MODE_CONNECTED) {
		if (conn->count_modes != 0) {
			int j;
			for(j = 0; j < conn->count_modes; j++)
				printf("    mode %2d: %-15s %dHz %ux%u\n", j, conn->modes[j].name, conn->modes[j].vrefresh, conn->modes[j].hdisplay,  conn->modes[j].vdisplay);
		}else
			printf("    no valid mode for connector %u\n", conn->connector_id);
	}else
		printf("    unused connector %u\n", conn->connector_id);

}

void drm_connectors(DrmToolArgs* args, drmModeConnector *conn)
{
	int i;

	print_drm_connector(args, conn);

	for(i=0; i < conn->count_props; i++){
		drmModePropertyPtr prop = drmModeGetProperty(args->drm_fd, conn->props[i]);
		if(!prop){
			printf("    cannot retrieve DRM property (connector_id: %d, prop_id: %d): errorno: %d (%s)\n", conn->connector_id, conn->props[i], errno, strerror(errno));
			continue;
		}

		drm_set_property(args, conn, prop, conn->prop_values[i]);

		drmModeFreeProperty(prop);
	}
}

void drm_resources(DrmToolArgs* args)
{
	drmModeRes *res;

	if(!(res = drmModeGetResources(args->drm_fd))){
		printf("  cannot retrieve DRM resources errno: %d (%s)\n", errno, strerror(errno));
		return;
	}

	int i;
	for(i=0; i < res->count_connectors; ++i){
		if(args->cmd == DRM_TOOL_SET && args->connector_id != res->connectors[i])
			continue;

		drmModeConnector *conn = drmModeGetConnector(args->drm_fd, res->connectors[i]);

		if(!conn){
			printf("  cannot retrieve DRM connector %u:%u errno: %d (%s)\n", i, res->connectors[i], errno, strerror(errno));
			continue;
		}

		drm_connectors(args, conn);
		drmModeFreeConnector(conn);
	}

	drmModeFreeResources(res);
}

void print_drm_capability(DrmToolArgs* args, const char* name, int capability)
{
	uint64_t value = -1;
	int ret;

	if((ret = drmGetCap(args->drm_fd, capability, &value) < 0)) {
		printf(" error getting capability '%s' (%d) ret: %d errno: %d (%s)\n", name, capability, ret, errno, strerror(errno));
		return;
	}

	printf(" capability '%s' (%d) : %" PRIu64 "\n", name, capability, value);
}

void print_drm_card(DrmToolArgs* args)
{
	if(args->cmd != DRM_TOOL_LIST)
		return;

	printf("Card %s\n", args->card);
	print_drm_capability(args, "DUMP BUFFER", DRM_CAP_DUMB_BUFFER);
	print_drm_capability(args, "VBLANK HIGH CRTC", DRM_CAP_VBLANK_HIGH_CRTC);
	print_drm_capability(args, "DUMB PREFERED DEPTH", DRM_CAP_DUMB_PREFERRED_DEPTH);
	print_drm_capability(args, "DUMB PREFER SHADOW", DRM_CAP_DUMB_PREFER_SHADOW);
	print_drm_capability(args, "PRIME", DRM_CAP_PRIME);
	print_drm_capability(args, "TIMESTAMP MONOTONIC", DRM_CAP_TIMESTAMP_MONOTONIC);
}

int main(int argc, const char* argv[])
{
	DrmToolArgs args;
	memset(&args, 0, sizeof(DrmToolArgs));

	args.cmd = DRM_TOOL_LIST;
	args.card = NULL;
	args.prop_name = NULL;

	if(argc < 2)
		return usage(argv[0]);

	if(strcmp(argv[1], "set") == 0){
		args.cmd = DRM_TOOL_SET;
		if(argc < 6)
			return usage(argv[0]);

		args.card = argv[2];
		args.connector_id = atoi(argv[3]);
		args.prop_name = argv[4];
		args.value = atoi(argv[5]);
	}

	while((args.card = get_card_name(&args))){
		if( (args.drm_fd = open(args.card, O_RDWR | O_CLOEXEC)) < 0){
			printf("Cannot open card '%s' :: errno: %d (%s)\n", args.card, errno, strerror(errno));
			continue;
		}

		print_drm_card(&args);

		drm_resources(&args);

		close(args.drm_fd);
		args.drm_fd = 0;
		args.card = NULL;
	}

	close(args.drm_fd);

	return 0;
}
