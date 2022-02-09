
#include <linux/input.h> //For LED
#include <linux/usb.h> //For USB_DIR_OUT

#include "hid.h"
#include "kmoip.h"
#include <asm/unaligned.h>

//static struct hid_device *ast_kbd0_hid = NULL;
//static struct hid_device *ast_ms0_hid = NULL;
//static unsigned int kbd0_event_pending = 0;
//static unsigned int ms0_event_pending = 0;

/* Generic Desktop Usage Page (0x01) */
#define HID_USG01_X           0x30
#define HID_USG01_Y           0x31
#define HID_USG01_WHEEL  0x38
#define HID_USG01_SYS_CTRL  0x80
#define HID_USG01_MAX    0xFF
#define HID_USG01_MIN    0


/* Keyboard/Keypad Usage Page (0x07) */
#define HID_USG07_RSRVD  0x00
#define HID_USG07_LCTRL   0xE0
#define HID_USG07_RGUI   0xE7
#define HID_USG07_MAX    HID_USG07_RGUI
#define HID_USG07_MIN    HID_USG07_RSRVD
#define HID_USG07_MODMAX    HID_USG07_RGUI
#define HID_USG07_MODMIN    HID_USG07_LCTRL
/* Button Usage Page (0x09) */
#define HID_USG09_NONE  0x00
#define HID_USG09_BTN01 0x01
#define HID_USG09_BTN08 0x08
#define HID_USG09_MIN HID_USG09_BTN01
#define HID_USG09_MAX HID_USG09_BTN08

#define HID_USG0C_MAX    0x3FF
#define HID_USG0C_MIN    0

#define HID_USG0D_DIGITIZER      (0x000D0001)
#define HID_USG0D_PEN            (0x000D0002)
#define HID_USG0D_LIGHTPEN       (0x000D0003)
#define HID_USG0D_TOUCHSCREEN    (0x000D0004)
#define HID_USG0D_TOUCHPAD       (0x000D0005)
#define HID_USG0D_WHITEBOARD     (0x000D0006)
#define HID_USG0D_MULTIDIGITZR   (0x000D000C)

/**
 * uerr - print error messages
 * @fmt:
 * @args:
 */
#define uerr(fmt, args...)						\
	do {								\
		printk(KERN_ERR "%-10s: ***ERROR*** %s(%d): " fmt,	\
			(in_interrupt() ? "interrupt" : (current)->comm),\
			__FUNCTION__, __LINE__,  ##args);	\
	} while(0)

/**
 * uinfo - print information messages
 * @fmt:
 * @args:
 */
#define uinfo(fmt, args...)					\
	do {							\
		printk(KERN_INFO "KMoIP: " fmt , ## args);	\
	} while(0)

#undef dbg
#define dbg uinfo

/*
 * Register a new report for a device.
 */

static struct hid_report *hid_register_report(struct hid_device *device, unsigned type, unsigned id)
{
	struct hid_report_enum *report_enum = device->report_enum + type;
	struct hid_report *report;

	if (report_enum->report_id_hash[id])
		return report_enum->report_id_hash[id];

	if (!(report = kmalloc(sizeof(struct hid_report), GFP_KERNEL)))
		return NULL;
	memset(report, 0, sizeof(struct hid_report));

	if (id != 0)
		report_enum->numbered = 1;

	report->id = id;
	report->type = type;
	report->size = 0;
	report->device = device;
	report_enum->report_id_hash[id] = report;

	list_add_tail(&report->list, &report_enum->report_list);

	return report;
}

/*
 * Register a new field for this report.
 */

static struct hid_field *hid_register_field(struct hid_report *report, unsigned usages, unsigned values)
{
	struct hid_field *field;

	if (report->maxfield == HID_MAX_FIELDS) {
		dbg("too many fields in report");
		return NULL;
	}

	if (!(field = kmalloc(sizeof(struct hid_field) + usages * sizeof(struct hid_usage)
		+ values * sizeof(unsigned), GFP_KERNEL))) return NULL;

	memset(field, 0, sizeof(struct hid_field) + usages * sizeof(struct hid_usage)
		+ values * sizeof(unsigned));

	field->index = report->maxfield++;
	report->field[field->index] = field;
	field->usage = (struct hid_usage *)(field + 1);
	field->value = (unsigned *)(field->usage + usages);
	field->report = report;

	return field;
}

/*
 * Open a collection. The type/usage is pushed on the stack.
 */

static int open_collection(struct hid_parser *parser, unsigned type)
{
	struct hid_collection *collection;
	unsigned usage;

	usage = parser->local.usage[0];

	if (parser->collection_stack_ptr == HID_COLLECTION_STACK_SIZE) {
		dbg("collection stack overflow");
		return -1;
	}

	if (parser->device->maxcollection == parser->device->collection_size) {
		collection = kmalloc(sizeof(struct hid_collection) *
				parser->device->collection_size * 2, GFP_KERNEL);
		if (collection == NULL) {
			dbg("failed to reallocate collection array");
			return -1;
		}
		memcpy(collection, parser->device->collection,
			sizeof(struct hid_collection) *
			parser->device->collection_size);
		memset(collection + parser->device->collection_size, 0,
			sizeof(struct hid_collection) *
			parser->device->collection_size);
		kfree(parser->device->collection);
		parser->device->collection = collection;
		parser->device->collection_size *= 2;
	}

	parser->collection_stack[parser->collection_stack_ptr++] =
		parser->device->maxcollection;

	collection = parser->device->collection +
		parser->device->maxcollection++;
	collection->type = type;
	collection->usage = usage;
	collection->level = parser->collection_stack_ptr - 1;

	if (type == HID_COLLECTION_APPLICATION)
		parser->device->maxapplication++;

	return 0;
}

/*
 * Close a collection.
 */

static int close_collection(struct hid_parser *parser)
{
	if (!parser->collection_stack_ptr) {
		dbg("collection stack underflow");
		return -1;
	}
	parser->collection_stack_ptr--;
	return 0;
}

/*
 * Climb up the stack, search for the specified collection type
 * and return the usage.
 */

static unsigned hid_lookup_collection(struct hid_parser *parser, unsigned type)
{
	int n;
	for (n = parser->collection_stack_ptr - 1; n >= 0; n--)
		if (parser->device->collection[parser->collection_stack[n]].type == type)
			return parser->device->collection[parser->collection_stack[n]].usage;
	return 0; /* we know nothing about this usage type */
}

/*
 * Add a usage to the temporary parser table.
 */

static int hid_add_usage(struct hid_parser *parser, unsigned usage)
{
	if (parser->local.usage_index >= HID_MAX_USAGES) {
		dbg("usage index exceeded");
		return -1;
	}
	parser->local.usage[parser->local.usage_index] = usage;
	parser->local.collection_index[parser->local.usage_index] =
		parser->collection_stack_ptr ?
		parser->collection_stack[parser->collection_stack_ptr - 1] : 0;
	parser->local.usage_index++;
	return 0;
}

/*
 * Register a new field for this report.
 */

static int hid_add_field(struct hid_parser *parser, unsigned report_type, unsigned flags)
{
	struct hid_report *report;
	struct hid_field *field;
	int usages;
	unsigned offset;
	int i;

	if (!(report = hid_register_report(parser->device, report_type, parser->global.report_id))) {
		dbg("hid_register_report failed");
		return -1;
	}

	if (parser->global.logical_maximum < parser->global.logical_minimum) {
		dbg("logical range invalid %d %d", parser->global.logical_minimum, parser->global.logical_maximum);
		return -1;
	}

	offset = report->size;
	report->size += parser->global.report_size * parser->global.report_count;

	if (!parser->local.usage_index) /* Ignore padding fields */
		return 0;

	usages = max_t(int, parser->local.usage_index, parser->global.report_count);

	if ((field = hid_register_field(report, usages, parser->global.report_count)) == NULL)
		return 0;

	field->physical = hid_lookup_collection(parser, HID_COLLECTION_PHYSICAL);
	field->logical = hid_lookup_collection(parser, HID_COLLECTION_LOGICAL);
	field->application = hid_lookup_collection(parser, HID_COLLECTION_APPLICATION);

	for (i = 0; i < usages; i++) {
		int j = i;
		/* Duplicate the last usage we parsed if we have excess values */
		if (i >= parser->local.usage_index)
			j = parser->local.usage_index - 1;
		field->usage[i].hid = parser->local.usage[j];
		field->usage[i].collection_index =
			parser->local.collection_index[j];
	}

	field->maxusage = usages;
	field->flags = flags;
	field->report_offset = offset;
	field->report_type = report_type;
	field->report_size = parser->global.report_size;
	field->report_count = parser->global.report_count;
	field->logical_minimum = parser->global.logical_minimum;
	field->logical_maximum = parser->global.logical_maximum;
	field->physical_minimum = parser->global.physical_minimum;
	field->physical_maximum = parser->global.physical_maximum;
	field->unit_exponent = parser->global.unit_exponent;
	field->unit = parser->global.unit;

	return 0;
}

/*
 * Read data value from item.
 */

static __inline__ __u32 item_udata(struct hid_item *item)
{
	switch (item->size) {
		case 1: return item->data.u8;
		case 2: return item->data.u16;
		case 4: return item->data.u32;
	}
	return 0;
}

static __inline__ __s32 item_sdata(struct hid_item *item)
{
	switch (item->size) {
		case 1: return item->data.s8;
		case 2: return item->data.s16;
		case 4: return item->data.s32;
	}
	return 0;
}

/*
 * Process a global item.
 */

static int hid_parser_global(struct hid_parser *parser, struct hid_item *item)
{
	switch (item->tag) {

		case HID_GLOBAL_ITEM_TAG_PUSH:

			if (parser->global_stack_ptr == HID_GLOBAL_STACK_SIZE) {
				dbg("global enviroment stack overflow");
				return -1;
			}

			memcpy(parser->global_stack + parser->global_stack_ptr++,
				&parser->global, sizeof(struct hid_global));
			return 0;

		case HID_GLOBAL_ITEM_TAG_POP:

			if (!parser->global_stack_ptr) {
				dbg("global enviroment stack underflow");
				return -1;
			}

			memcpy(&parser->global, parser->global_stack + --parser->global_stack_ptr,
				sizeof(struct hid_global));
			return 0;

		case HID_GLOBAL_ITEM_TAG_USAGE_PAGE:
			parser->global.usage_page = item_udata(item);
			return 0;

		case HID_GLOBAL_ITEM_TAG_LOGICAL_MINIMUM:
			parser->global.logical_minimum = item_sdata(item);
			return 0;

		case HID_GLOBAL_ITEM_TAG_LOGICAL_MAXIMUM:
			if (parser->global.logical_minimum < 0)
				parser->global.logical_maximum = item_sdata(item);
			else
				parser->global.logical_maximum = item_udata(item);
			return 0;

		case HID_GLOBAL_ITEM_TAG_PHYSICAL_MINIMUM:
			parser->global.physical_minimum = item_sdata(item);
			return 0;

		case HID_GLOBAL_ITEM_TAG_PHYSICAL_MAXIMUM:
			if (parser->global.physical_minimum < 0)
				parser->global.physical_maximum = item_sdata(item);
			else
				parser->global.physical_maximum = item_udata(item);
			return 0;

		case HID_GLOBAL_ITEM_TAG_UNIT_EXPONENT:
			parser->global.unit_exponent = item_sdata(item);
			return 0;

		case HID_GLOBAL_ITEM_TAG_UNIT:
			parser->global.unit = item_udata(item);
			return 0;

		case HID_GLOBAL_ITEM_TAG_REPORT_SIZE:
			if ((parser->global.report_size = item_udata(item)) > 32) {
				dbg("invalid report_size %d", parser->global.report_size);
				return -1;
			}
			return 0;

		case HID_GLOBAL_ITEM_TAG_REPORT_COUNT:
			if ((parser->global.report_count = item_udata(item)) > HID_MAX_USAGES) {
				dbg("invalid report_count %d", parser->global.report_count);
				return -1;
			}
			return 0;

		case HID_GLOBAL_ITEM_TAG_REPORT_ID:
			if ((parser->global.report_id = item_udata(item)) == 0) {
				dbg("report_id 0 is invalid");
				return -1;
			}
			return 0;

		default:
			dbg("unknown global tag 0x%x", item->tag);
			return -1;
	}
}

/*
 * Process a local item.
 */

static int hid_parser_local(struct hid_parser *parser, struct hid_item *item)
{
	__u32 data;
	unsigned n;

	if (item->size == 0) {
		dbg("item data expected for local item");
		return -1;
	}

	data = item_udata(item);

	switch (item->tag) {

		case HID_LOCAL_ITEM_TAG_DELIMITER:

			if (data) {
				/*
				 * We treat items before the first delimiter
				 * as global to all usage sets (branch 0).
				 * In the moment we process only these global
				 * items and the first delimiter set.
				 */
				if (parser->local.delimiter_depth != 0) {
					dbg("nested delimiters");
					return -1;
				}
				parser->local.delimiter_depth++;
				parser->local.delimiter_branch++;
			} else {
				if (parser->local.delimiter_depth < 1) {
					dbg("bogus close delimiter");
					return -1;
				}
				parser->local.delimiter_depth--;
			}
			return 1;

		case HID_LOCAL_ITEM_TAG_USAGE:

			if (parser->local.delimiter_branch > 1) {
				dbg("alternative usage ignored");
				return 0;
			}

			if (item->size <= 2)
				data = (parser->global.usage_page << 16) + data;

			return hid_add_usage(parser, data);

		case HID_LOCAL_ITEM_TAG_USAGE_MINIMUM:

			if (parser->local.delimiter_branch > 1) {
				dbg("alternative usage ignored");
				return 0;
			}

			if (item->size <= 2)
				data = (parser->global.usage_page << 16) + data;

			parser->local.usage_minimum = data;
			return 0;

		case HID_LOCAL_ITEM_TAG_USAGE_MAXIMUM:

			if (parser->local.delimiter_branch > 1) {
				dbg("alternative usage ignored");
				return 0;
			}

			if (item->size <= 2)
				data = (parser->global.usage_page << 16) + data;

			for (n = parser->local.usage_minimum; n <= data; n++)
				if (hid_add_usage(parser, n)) {
					dbg("hid_add_usage failed\n");
					return -1;
				}
			return 0;

		default:

			dbg("unknown local item tag 0x%x", item->tag);
			return 0;
	}
	return 0;
}

/*
 * Process a main item.
 */

static int hid_parser_main(struct hid_parser *parser, struct hid_item *item)
{
	__u32 data;
	int ret;

	data = item_udata(item);

	switch (item->tag) {
		case HID_MAIN_ITEM_TAG_BEGIN_COLLECTION:
			ret = open_collection(parser, data & 0xff);
			break;
		case HID_MAIN_ITEM_TAG_END_COLLECTION:
			ret = close_collection(parser);
			break;
		case HID_MAIN_ITEM_TAG_INPUT:
			ret = hid_add_field(parser, HID_INPUT_REPORT, data);
			break;
		case HID_MAIN_ITEM_TAG_OUTPUT:
			ret = hid_add_field(parser, HID_OUTPUT_REPORT, data);
			break;
		case HID_MAIN_ITEM_TAG_FEATURE:
			ret = hid_add_field(parser, HID_FEATURE_REPORT, data);
			break;
		default:
			dbg("unknown main item tag 0x%x", item->tag);
			ret = 0;
	}

	memset(&parser->local, 0, sizeof(parser->local));	/* Reset the local parser environment */

	return ret;
}

/*
 * Process a reserved item.
 */

static int hid_parser_reserved(struct hid_parser *parser, struct hid_item *item)
{
	dbg("reserved item type, tag 0x%x", item->tag);
	return 0;
}

/*
 * Free a report and all registered fields. The field->usage and
 * field->value table's are allocated behind the field, so we need
 * only to free(field) itself.
 */

static void hid_free_report(struct hid_report *report)
{
	unsigned n;

	for (n = 0; n < report->maxfield; n++)
		kfree(report->field[n]);
	kfree(report);
}

/*
 * Free a device structure, all reports, and all fields.
 */

static void hid_free_device(struct hid_device *device)
{
	unsigned i,j;

	hid_ff_exit(device);

	for (i = 0; i < HID_REPORT_TYPES; i++) {
		struct hid_report_enum *report_enum = device->report_enum + i;

		for (j = 0; j < 256; j++) {
			struct hid_report *report = report_enum->report_id_hash[j];
			if (report)
				hid_free_report(report);
		}
	}

	kfree(device->rdesc);
	kfree(device);
}

/*
 * Fetch a report description item from the data stream. We support long
 * items, though they are not used yet.
 */

static u8 *fetch_item(__u8 *start, __u8 *end, struct hid_item *item)
{
	u8 b;

	if ((end - start) <= 0)
		return NULL;

	b = *start++;

	item->type = (b >> 2) & 3;
	item->tag  = (b >> 4) & 15;

	if (item->tag == HID_ITEM_TAG_LONG) {

		item->format = HID_ITEM_FORMAT_LONG;

		if ((end - start) < 2)
			return NULL;

		item->size = *start++;
		item->tag  = *start++;

		if ((end - start) < item->size)
			return NULL;

		item->data.longdata = start;
		start += item->size;
		return start;
	}

	item->format = HID_ITEM_FORMAT_SHORT;
	item->size = b & 3;

	switch (item->size) {

		case 0:
			return start;

		case 1:
			if ((end - start) < 1)
				return NULL;
			item->data.u8 = *start++;
			return start;

		case 2:
			if ((end - start) < 2)
				return NULL;
			item->data.u16 = le16_to_cpu(get_unaligned((__le16*)start));
			start = (__u8 *)((__le16 *)start + 1);
			return start;

		case 3:
			item->size++;
			if ((end - start) < 4)
				return NULL;
			item->data.u32 = le32_to_cpu(get_unaligned((__le32*)start));
			start = (__u8 *)((__le32 *)start + 1);
			return start;
	}

	return NULL;
}


/*
 * Traverse the supplied list of reports and find the longest
 */
static void hid_find_max_report(struct hid_device *hid, unsigned int type, int *max)
{
	struct hid_report *report;
	int size;

	list_for_each_entry(report, &hid->report_enum[type].report_list, list) {
		size = ((report->size - 1) >> 3) + 1;
		if (type == HID_INPUT_REPORT && hid->report_enum[type].numbered)
			size++;
		if (*max < size)
			*max = size;
	}
}


/*
 * Parse a report description into a hid_device structure. Reports are
 * enumerated, fields are attached to these reports.
 */

static struct hid_device *hid_parse_report(__u8 *start, unsigned size)
{
	struct hid_device *device;
	struct hid_parser *parser;
	struct hid_item item;
	__u8 *end;
	unsigned i;
	static int (*dispatch_type[])(struct hid_parser *parser,
				      struct hid_item *item) = {
		hid_parser_main,
		hid_parser_global,
		hid_parser_local,
		hid_parser_reserved
	};

	if (!(device = kmalloc(sizeof(struct hid_device), GFP_KERNEL)))
		return NULL;
	memset(device, 0, sizeof(struct hid_device));

	if (!(device->collection = kmalloc(sizeof(struct hid_collection) *
				   HID_DEFAULT_NUM_COLLECTIONS, GFP_KERNEL))) {
		kfree(device);
		return NULL;
	}
	memset(device->collection, 0, sizeof(struct hid_collection) *
		HID_DEFAULT_NUM_COLLECTIONS);
	device->collection_size = HID_DEFAULT_NUM_COLLECTIONS;

	for (i = 0; i < HID_REPORT_TYPES; i++)
		INIT_LIST_HEAD(&device->report_enum[i].report_list);

	if (!(device->rdesc = (__u8 *)kmalloc(size, GFP_KERNEL))) {
		kfree(device->collection);
		kfree(device);
		return NULL;
	}
	memcpy(device->rdesc, start, size);
	device->rsize = size;

	if (!(parser = kmalloc(sizeof(struct hid_parser), GFP_KERNEL))) {
		kfree(device->rdesc);
		kfree(device->collection);
		kfree(device);
		return NULL;
	}
	memset(parser, 0, sizeof(struct hid_parser));
	parser->device = device;

	end = start + size;
	while ((start = fetch_item(start, end, &item)) != NULL) {

		if (item.format != HID_ITEM_FORMAT_SHORT) {
			dbg("unexpected long global item");
			kfree(device->collection);
			hid_free_device(device);
			kfree(parser);
			return NULL;
		}

		if (dispatch_type[item.type](parser, &item)) {
			dbg("item %u %u %u %u parsing failed\n",
				item.format, (unsigned)item.size, (unsigned)item.type, (unsigned)item.tag);
			kfree(device->collection);
			hid_free_device(device);
			kfree(parser);
			return NULL;
		}

		if (start == end) {
			if (parser->collection_stack_ptr) {
				dbg("unbalanced collection at end of report description");
				kfree(device->collection);
				hid_free_device(device);
				kfree(parser);
				return NULL;
			}
			if (parser->local.delimiter_depth) {
				dbg("unbalanced delimiter at end of report description");
				kfree(device->collection);
				hid_free_device(device);
				kfree(parser);
				return NULL;
			}
			kfree(parser);
			return device;
		}
	}

	dbg("item fetching failed at offset %d\n", (int)(end - start));
	kfree(device->collection);
	hid_free_device(device);
	kfree(parser);
	return NULL;
}

/*
 * Convert a signed n-bit integer to signed 32-bit integer. Common
 * cases are done through the compiler, the screwed things has to be
 * done by hand.
 */

static __inline__ __s32 snto32(__u32 value, unsigned n)
{
	switch (n) {
		case 8:  return ((__s8)value);
		case 16: return ((__s16)value);
		case 32: return ((__s32)value);
	}
	return value & (1 << (n - 1)) ? value | (-1 << n) : value;
}

/*
 * Convert a signed 32-bit integer to a signed n-bit integer.
 */

static __inline__ __u32 s32ton(__s32 value, unsigned n)
{
	__s32 a = value >> (n - 1);
	if (a && a != -1)
		return value < 0 ? 1 << (n - 1) : (1 << (n - 1)) - 1;
	return value & ((1 << n) - 1);
}

/*
 * Extract/implement a data field from/to a report.
 */

static __inline__ __u32 extract(__u8 *report, unsigned offset, unsigned n)
{
	report += (offset >> 5) << 2; offset &= 31;
	return (le64_to_cpu(get_unaligned((__le64*)report)) >> offset) & ((1ULL << n) - 1);
}

static __inline__ void implement(__u8 *report, unsigned offset, unsigned n, __u32 value)
{
	report += (offset >> 5) << 2; offset &= 31;
	put_unaligned((get_unaligned((__le64*)report)
		& cpu_to_le64(~((((__u64) 1 << n) - 1) << offset)))
		| cpu_to_le64((__u64)value << offset), (__le64*)report);
}




struct hid_device *ast_create_hid_class_device(char rdesc[], int rsize, char bInterfaceNumber)
{
	struct hid_device *hid = NULL;
	int insize = 0;

#if 0
	int n;
	printk("report descriptor (size %u) = ", rsize);
	for (n = 0; n < rsize; n++)
		printk(" %02x", (unsigned char) rdesc[n]);
	printk("\n");
#endif

	if (!(hid = hid_parse_report(rdesc, rsize))) {
		dbg("parsing report descriptor failed");
		return NULL;
	}

	//hid->quirks = quirks;

	hid->bufsize = HID_MIN_BUFFER_SIZE;
	hid_find_max_report(hid, HID_INPUT_REPORT, &hid->bufsize);
	hid_find_max_report(hid, HID_OUTPUT_REPORT, &hid->bufsize);
	hid_find_max_report(hid, HID_FEATURE_REPORT, &hid->bufsize);

	if (hid->bufsize > HID_MAX_BUFFER_SIZE)
		hid->bufsize = HID_MAX_BUFFER_SIZE;

	hid_find_max_report(hid, HID_INPUT_REPORT, &insize);

	if (insize > HID_MAX_BUFFER_SIZE)
		insize = HID_MAX_BUFFER_SIZE;
#if 0
	if (hid_alloc_buffers(dev, hid)) {
		hid_free_buffers(dev, hid);
		goto fail;
	}
#endif

	hid->version = 0x0111; //HID 1.1
	hid->country = 0;
	hid->dev = NULL;
	hid->intf = NULL;
	hid->ifnum = bInterfaceNumber;

	switch (bInterfaceNumber) {
	case KBD0_INTF_NUM:
		strlcpy(hid->name, "AST Keyboard", sizeof(hid->name));
		break;
	case KBD1_INTF_NUM:
		strlcpy(hid->name, "AST Keyboard Extra", sizeof(hid->name));
		break;
	case MS0_INTF_NUM:
		strlcpy(hid->name, "AST Mouse", sizeof(hid->name));
		break;
	case TCH0_INTF_NUM:
		strlcpy(hid->name, "AST Absolute Mouse (Touch0)", sizeof(hid->name));
		break;
	default:
		BUG();
	}

	hid->uniq[0] = 0;

	return hid;

//fail:

	hid_free_device(hid);

	return NULL;
}

static int hidkm_set_field_value(struct hid_field *field, unsigned offset, __s32 value)
{
	//hid_dump_input(field->usage + value, 1);

	if (offset >= field->report_count) {
		dbg("offset (%d) exceeds report_count (%d)", offset, field->report_count);
		hid_dump_field(field, 8);
		return -1;
	}

	/* Bruce. There are mouses has report_size > 8bits.
	** Since the value is singed, we can't just take the lower 8bits as output.
	** From my test, cut to the boundary value performs much better than normalize the value.
	*/
	if (value > field->logical_maximum)
		value = field->logical_maximum;
	else if (value < field->logical_minimum)
		value = field->logical_minimum;

	field->value[offset] = value;
	return 0;
}

/*Copied from static void hid_output_field(struct hid_field *field, __u8 *data)*/
//Use hid_output_report()??
static int hidkm_set_report_field(struct hid_field *field, __u8 *report)
{
	unsigned count = field->report_count;
	unsigned offset = field->report_offset;
	unsigned size = field->report_size;
	unsigned n;

	for (n = 0; n < count; n++) {
		if (field->logical_minimum < 0)	/* signed values */
			implement(report, offset + n * size, size, s32ton(field->value[n], size));
		else				/* unsigned values */
			implement(report, offset + n * size, size, field->value[n]);
	}
	return 0;
}


static struct hid_usage *hidkm_find_usage(struct hid_field *field, unsigned int usage_id)
{
	int i;
	for (i = 0; i < field->maxusage; i++) {
		//printk("match usage[%d].hid(0x%08X) for usage id (0x%08X)\n", i, field->usage[i].hid, usage_id);
		if (field->usage[i].hid == usage_id)
			return &field->usage[i];
	}
	return NULL;
}

static struct hid_field *hidkm_find_field_by_usage(struct hid_device *hid, __u32 wanted_usage, int type)
{
	struct hid_report *report;
	struct hid_usage *usage = NULL;
	int i = 0;

	list_for_each_entry(report, &hid->report_enum[type].report_list, list) {
		for (i = 0; i < report->maxfield; i++) {
			//printk("match field[%d] for usage id (0x%08X)\n", i, wanted_usage);
			usage = hidkm_find_usage(report->field[i], wanted_usage);
			if (usage != NULL)
				return report->field[i];
		}
	}

	return NULL;
}

static inline void hidkm_reset_field_value(struct hid_field *field, __s32 value)
{
	int i;
	
	for (i = 0; i < field->report_count; i++) {
		hidkm_set_field_value(field, i, value);
	}
}

static inline void _add_into_array(__s32 *array, unsigned int array_size, __s32 value)
{
	int i;
	int first_null_idx = -1;
	int is_duplicated = 0;

	//Assume 0 as null value
	for (i = 0; i < array_size; i++) {
		if ((first_null_idx == -1) && (array[i] == 0)) {
			first_null_idx = i;
		} else if (array[i] == value) {
			is_duplicated = 1;
		}
	}
	if (!is_duplicated && (first_null_idx != -1))
		array[first_null_idx] = value;
}

static inline void _remove_from_array(__s32 *array, unsigned int array_size, __s32 value)
{
	int i, j;
	//Try to find matched. If found, remove it (inlcuding the place holder).
	for (i = 0; i < array_size; i++)
		if (array[i] == value) break;

	for (j = i; j < array_size; j++) {
		array[j] = ((j + 1) == array_size)?(0):(array[j+1]);
		if (array[j] == 0) break;
	}
}

static int hidkm_set_field_value_ex(struct hid_field *field, unsigned int usage_id, __s32 value)
{
	unsigned int offset;

	if (field->flags & HID_MAIN_ITEM_VARIABLE) {
		// variable type
		offset = usage_id - (field->usage[0].hid & HID_USAGE); //usage_id - min_usage
		//printk("offset = %d, value=%d usage id=0x%X\n", offset, value, usage_id);
		hidkm_set_field_value(field, offset, value);
	} else {
		//For array type
		offset = usage_id - field->logical_minimum;
		if (value) {
			_add_into_array(field->value, field->report_count, offset);
		} else {
			_remove_from_array(field->value, field->report_count, offset);
		}
	}
	return 0;
}

/* For (usage->hid & HID_USAGE_PAGE) == HID_UP_KEYBOARD */
static inline void process_up_keyboard(struct kmoip_drv *kmdrv, struct hid_field *field, struct hid_usage *usage, __s32 value)
{
	unsigned int usgid = usage->hid & HID_USAGE;
	struct hid_field *my_field = NULL;
	
	if (usgid < HID_USG07_MIN || usgid > HID_USG07_MAX) {
		//uerr("Usage ID out of supported range!(0x%02X)\n", usgid);
		return;
	}
	
	my_field = hidkm_find_field_by_usage(kmdrv->kbd0_hiddev, usage->hid, HID_INPUT_REPORT);
	if (my_field == NULL) {
		uerr("my_field not found?!usage id(0x%08X)\n", usage->hid);
		return;
	}
	hidkm_set_field_value_ex(my_field, usgid, value);
	kmdrv->kbd0_event_pending++;
}

static inline void process_mouse_button(struct kmoip_drv *kmdrv, struct hid_field *field, struct hid_usage *usage, __s32 value)
{
	unsigned int usgid = usage->hid & HID_USAGE;
	struct hid_field *my_field = NULL;
	roaming_s *roaming = &kmdrv->roaming;

	if (kmr_is_roaming_enabled(roaming)) {
		my_field = hidkm_find_field_by_usage(kmdrv->tch0_hiddev, usage->hid, HID_INPUT_REPORT);
		if (my_field == NULL) {
			uerr("my_field not found?!usage id(0x%08X)\n", usage->hid);
			return;
		}
		hidkm_set_field_value_ex(my_field, usgid, value);
		kmdrv->tch0_event_pending++;
	} else {
		my_field = hidkm_find_field_by_usage(kmdrv->ms0_hiddev, usage->hid, HID_INPUT_REPORT);
		if (my_field == NULL) {
			uerr("my_field not found?!usage id(0x%08X)\n", usage->hid);
			return;
		}
		hidkm_set_field_value_ex(my_field, usgid, value);
		kmdrv->ms0_event_pending++;
	}
}

/* For (usage->hid & HID_USAGE_PAGE) == HID_UP_BUTTON */
/* for mouse buttons */
static inline void process_up_button(struct kmoip_drv *kmdrv, struct hid_field *field, struct hid_usage *usage, __s32 value)
{
	unsigned int usgid = usage->hid & HID_USAGE;
	struct hid_field *my_field = NULL;
	
	if (usgid < HID_USG09_MIN || usgid > HID_USG09_MAX) {
		//uerr("Usage ID out of supported range!(0x%02X)\n", usgid);
		return;
	}
	
	switch(field->application) {
		case HID_GD_POINTER:
			my_field = hidkm_find_field_by_usage(kmdrv->tch0_hiddev, usage->hid, HID_INPUT_REPORT);
			if (my_field == NULL) {
				uerr("my_field not found?!usage id(0x%08X)\n", usage->hid);
				break;
			}
			hidkm_set_field_value_ex(my_field, usgid, value);
			kmdrv->tch0_event_pending++;
			break;
		case HID_GD_MOUSE:
			/* default to mouse. */
		default:
			process_mouse_button(kmdrv, field, usage, value);
			break;
	}
}

static inline unsigned process_mouse_roaming_pointer(struct kmoip_drv *kmdrv, struct hid_field *field, struct hid_usage *usage, __s32 value)
{
	unsigned int usgid;
	struct hid_field *my_field;
	roaming_s *roaming = &kmdrv->roaming;
	kmr_target_s *target;
	kmr_csr_s *csr;

	if (!kmr_is_roaming_enabled(roaming))
		goto not_interested;

	target = kmr_get_cur_target(roaming);

#if 0
	/*
	 * Bruce191030. blocking inactive target will also block
	 * roaming detection which will stop user from roaming
	 * from an inactive target to others. It is bad.
	 * To resolve this issue, we won't block it.
	 * Add default active zone value to let user roaming through it.
	 */
	if (unlikely(!kmr_target_got_active_zone(target)))
		goto ignore;
#endif

	if (unlikely(target->work_state != KMRWS_SCKT_READY))
		goto ignore;

	my_field = hidkm_find_field_by_usage(kmdrv->tch0_hiddev, usage->hid, HID_INPUT_REPORT);
	if (my_field == NULL) {
		uerr("my_field not found?!usage id(0x%08X)\n", usage->hid);
		goto not_interested;
	}

	usgid = usage->hid & HID_USAGE;
	if (usgid == HID_USG01_X)
		csr = &target->x;
	else
		csr = &target->y;

	/* map cursor value to active zone abs position and then to tch0 abs position */
	if (field->flags & HID_MAIN_ITEM_RELATIVE)
		value = kmr_csr_remap_from_rel(
		                               roaming,
		                               csr,
		                               value,
		                               (my_field->logical_maximum - my_field->logical_minimum));
	else
		value = kmr_csr_remap_from_abs(
		                               roaming,
		                               csr,
		                               value,
		                               (my_field->logical_maximum - my_field->logical_minimum),
		                               (field->logical_maximum - field->logical_minimum));

	hidkm_set_field_value_ex(my_field, usgid, value);
	kmdrv->tch0_event_pending++;
ignore:
	return 1;
not_interested:
	return 0;
}

static inline void process_mouse_pointer(struct kmoip_drv *kmdrv, struct hid_field *field, struct hid_usage *usage, __s32 value)
{
	unsigned int usgid;
	struct hid_field *my_field;
	static int ms0_x_need_cleanup = 0, ms0_y_need_cleanup = 0;

	switch (field->application) {
		case HID_GD_POINTER: /* Not in spec, but in real world. */
		case HID_GD_MOUSE:
			break;
		case HID_USG0D_DIGITIZER:
		case HID_USG0D_PEN:
		case HID_USG0D_LIGHTPEN:
		case HID_USG0D_TOUCHSCREEN:
		case HID_USG0D_TOUCHPAD:
		case HID_USG0D_WHITEBOARD:
		case HID_USG0D_MULTIDIGITZR:
		default:
			/* Ignore non-mouse type for now. TBD. */
			return;
	}

	if (process_mouse_roaming_pointer(kmdrv, field, usage, value)) {
		if (likely((!ms0_x_need_cleanup) && (!ms0_y_need_cleanup)))
			return;
		/* Clean up ms0 pointer x/y value. Otherwise, wheel/button report will include it. */
		if (field->flags & HID_MAIN_ITEM_RELATIVE) {
			usgid = usage->hid & HID_USAGE;
			my_field = hidkm_find_field_by_usage(kmdrv->ms0_hiddev, usage->hid, HID_INPUT_REPORT);
			if (!my_field)
				return;

			hidkm_set_field_value_ex(my_field, usgid, 0);
			/* We assume ONLY HID_USG01_X and HID_USG01_Y usgid will be passed into this func. */
			if (usgid == HID_USG01_X)
				ms0_x_need_cleanup = 0;
			else
				ms0_y_need_cleanup = 0;
		}
		return;
	}

	/* Normal non-roaming case. */
	usgid = usage->hid & HID_USAGE;

	if (field->flags & HID_MAIN_ITEM_RELATIVE) {
		if (usgid == HID_USG01_X)
			ms0_x_need_cleanup = 1;
		else
			ms0_y_need_cleanup = 1;

		my_field = hidkm_find_field_by_usage(kmdrv->ms0_hiddev, usage->hid, HID_INPUT_REPORT);
		if (my_field == NULL) {
			uerr("my_field not found?!usage id(0x%08X)\n", usage->hid);
			return;
		}
		hidkm_set_field_value_ex(my_field, usgid, value);
		kmdrv->ms0_event_pending++;
	} else {
		my_field = hidkm_find_field_by_usage(kmdrv->tch0_hiddev, usage->hid, HID_INPUT_REPORT);
		if (my_field == NULL) {
			uerr("my_field not found?!usage id(0x%08X)\n", usage->hid);
			return;
		}
		/* Normalize absolute X/Y. */
		value = value * (my_field->logical_maximum - my_field->logical_minimum) / (field->logical_maximum - field->logical_minimum);
		hidkm_set_field_value_ex(my_field, usgid, value);
		kmdrv->tch0_event_pending++;
	}
}

static inline void process_mouse_wheel(struct kmoip_drv *kmdrv, struct hid_field *field, struct hid_usage *usage, __s32 value)
{
	unsigned int usgid = usage->hid & HID_USAGE;
	struct hid_field *my_field = NULL;
	roaming_s *roaming = &kmdrv->roaming;

	if (kmr_is_roaming_enabled(roaming)) {
		my_field = hidkm_find_field_by_usage(kmdrv->tch0_hiddev, usage->hid, HID_INPUT_REPORT);
		if (my_field == NULL) {
			uerr("my_field not found?!usage id(0x%08X)\n", usage->hid);
			return;
		}
		hidkm_set_field_value_ex(my_field, usgid, value);
		kmdrv->tch0_event_pending++;
	} else {
		my_field = hidkm_find_field_by_usage(kmdrv->ms0_hiddev, usage->hid, HID_INPUT_REPORT);
		if (my_field == NULL) {
			uerr("my_field not found?!usage id(0x%08X)\n", usage->hid);
			return;
		}
		hidkm_set_field_value_ex(my_field, usgid, value);
		kmdrv->ms0_event_pending++;
	}
}

/* For (usage->hid & HID_USAGE_PAGE) == HID_UP_GENDESK */
/* for mouse pointers, wheel or kbd1 report ID 3 system control */
static inline void process_up_gendesk(struct kmoip_drv *kmdrv, struct hid_field *field, struct hid_usage *usage, __s32 value)
{
	unsigned int usgid = usage->hid & HID_USAGE;
	struct hid_field *my_field = NULL;

	switch (usgid) {
	case HID_USG01_SYS_CTRL: // for kbd1 report ID 3 //system control
		if (usgid < HID_USG01_MIN || usgid > HID_USG01_MAX) {
			//uerr("Usage ID out of supported range!(0x%02X)\n", usgid);
			break;
		}
		my_field = hidkm_find_field_by_usage(kmdrv->kbd1_hiddev, usage->hid, HID_INPUT_REPORT);
		if (my_field == NULL) {
			uerr("my_field not found?!usage id(0x%08X)\n", usage->hid);
			break;
		}
		hidkm_set_field_value_ex(my_field, usgid, value);
		kmdrv->kbd1_3_event_pending++;

		break;
	case HID_USG01_WHEEL: //for mouse wheel
		process_mouse_wheel(kmdrv, field, usage, value);
		break;
	case HID_USG01_X: case HID_USG01_Y:  //for mouse pointer
		process_mouse_pointer(kmdrv, field, usage, value);
		break;
	default:
		break;
	}
}

/* For (usage->hid & HID_USAGE_PAGE) == HID_UP_CONSUMER */
/* for kbd1 report ID 1 */
static inline void process_up_consumer(struct kmoip_drv *kmdrv, struct hid_field *field, struct hid_usage *usage, __s32 value)
{
	unsigned int usgid = usage->hid & HID_USAGE;
	struct hid_field *my_field = NULL;
	
	if (usgid < HID_USG0C_MIN || usgid > HID_USG0C_MAX) {
		//uerr("Usage ID out of supported range!(0x%02X)\n", usgid);
		return;
	}
	
	my_field = hidkm_find_field_by_usage(kmdrv->kbd1_hiddev, usage->hid, HID_INPUT_REPORT);
	if (my_field == NULL) {
		uerr("my_field not found?!usage id(0x%08X)\n", usage->hid);
		return;
	}
	hidkm_set_field_value_ex(my_field, usgid, value);
	kmdrv->kbd1_1_event_pending++;
}

int hidkm_process_event(struct hid_device *hid, struct hid_field *field, struct hid_usage *usage, __s32 value, struct pt_regs *regs)
{
	struct kmoip_drv *kmdrv = hid->kmdrv;

	if (!kmdrv)
		return 0;

	//printk("field->app=0x%08X, hid=0x%08X\n", field->application, usage->hid);
	//See whether we are interests in the reported usage code
	switch (usage->hid & HID_USAGE_PAGE) {
		case HID_UP_KEYBOARD:
		{
			process_up_keyboard(kmdrv, field, usage, value);
			break;
		}
		case HID_UP_BUTTON: //for mouse buttons:
		{
			process_up_button(kmdrv, field, usage, value);
			break;
		}
		case HID_UP_GENDESK: //for mouse pointers, wheel or kbd1 report ID 3 system control
		{
			process_up_gendesk(kmdrv, field, usage, value);
			break;
		}
		case HID_UP_CONSUMER: //for kbd1 report ID 1
		{
			process_up_consumer(kmdrv, field, usage, value);
			break;
		}
		default:
			//printk("Ignore non-interested event\n");
			break;
	}

	if (!kmdrv->is_socket_ready)
		return 0;

	return 1; //don't pass to input subsystem
}

static void _hidkm_fill_report_event_kbd0(struct kmoip_drv *kmdrv, kmr_target_s *target, struct kmoip_report *report)
{
	__u8 *report_buf = report->report;
	struct hid_field *my_field = NULL;

	kmdrv->kbd0_event_pending = 0;

	memset(report_buf, 0, KMOIP_MAX_REPORT_BUF_SIZE);

	//printk("report->type=%x\n", report->type);

	my_field = hidkm_find_field_by_usage(kmdrv->kbd0_hiddev, (__u32)(HID_UP_KEYBOARD|HID_USG07_MODMIN), HID_INPUT_REPORT);
	if (my_field == NULL) {
		uerr("Can't find my_field? (modifier)\n");
		return;
	}
	hidkm_set_report_field(my_field, report_buf);

	my_field = hidkm_find_field_by_usage(kmdrv->kbd0_hiddev, (__u32)(HID_UP_KEYBOARD|HID_USG07_MIN), HID_INPUT_REPORT);
	if (my_field == NULL) {
		uerr("Can't find my_field (keycode)?\n");
		return;
	}
	hidkm_set_report_field(my_field, report_buf);

	report->magic_num = KMOIP_MAGIC_NUM;
	report->from = kmdrv->id;
	report->to = target->host_id;
	report->seq_num = kmdrv->seq_num++;
	report->context = (void *)kmdrv->token_interval;
	report->report_id = 0;
	report->report_type = HID_REPORT_TYPE_INPUT;
	report->interface = KBD0_INTF_NUM;
	report->report_length = ((my_field->report->size - 1) >> 3) + 1; //The answer should be 8 bytes. This is a simplified method.
#if 0
	if (0) {
		int i;
		printk("kbd0 report_buf dump = ");
		for (i = 0; i < report->report_length; i++) {
			printk(" %02X", report_buf[i]);
		}
		printk("\n");
	}
#endif
}

static void _hidkm_fill_report_event_kbd1_1(struct kmoip_drv *kmdrv, kmr_target_s *target, struct kmoip_report *report)
{
	__u8 *report_buf = report->report;
	struct hid_field *my_field = NULL;

	kmdrv->kbd1_1_event_pending = 0;

	memset(report_buf, 0, KMOIP_MAX_REPORT_BUF_SIZE);

	//printk("report->type=%x\n", report->type);

	report_buf[0] = 1; //set report id to 1 for HID_UP_CONSUMER

	my_field = hidkm_find_field_by_usage(kmdrv->kbd1_hiddev, (__u32)(HID_UP_CONSUMER|HID_USG0C_MIN), HID_INPUT_REPORT);
	if (my_field == NULL) {
		uerr("Can't find my_field? (modifier)\n");
		return;
	}
	hidkm_set_report_field(my_field, &report_buf[1]);

	report->magic_num = KMOIP_MAGIC_NUM;
	report->from = kmdrv->id;
	report->to = target->host_id;
	report->seq_num = kmdrv->seq_num++;
	report->context = (void *)kmdrv->token_interval;
	report->report_id = 1;
	report->report_type = HID_REPORT_TYPE_INPUT;
	report->interface = KBD1_INTF_NUM;
	report->report_length = 1 + ((my_field->report->size - 1) >> 3) + 1; //The answer should be 8 bytes. This is a simplified method.
#if 0
	if (0) {
		int i;
		printk("kbd1_1 report_buf dump = ");
		for (i = 0; i < report->report_length; i++) {
			printk(" %02X", report_buf[i]);
		}
		printk("\n");
	}
#endif
}

static void _hidkm_fill_report_event_kbd1_3(struct kmoip_drv *kmdrv, kmr_target_s *target, struct kmoip_report *report)
{
	__u8 *report_buf = report->report;
	struct hid_field *my_field = NULL;

	kmdrv->kbd1_3_event_pending = 0;

	memset(report_buf, 0, KMOIP_MAX_REPORT_BUF_SIZE);

	//printk("report->type=%x\n", report->type);

	report_buf[0] = 3; //set report id to 3 for HID_UP_GENDESK|system control

	my_field = hidkm_find_field_by_usage(kmdrv->kbd1_hiddev, (__u32)(HID_UP_GENDESK|HID_USG01_MIN), HID_INPUT_REPORT);
	if (my_field == NULL) {
		uerr("Can't find my_field? (modifier)\n");
		return;
	}
	hidkm_set_report_field(my_field, &report_buf[1]);

	report->magic_num = KMOIP_MAGIC_NUM;
	report->from = kmdrv->id;
	report->to = target->host_id;
	report->seq_num = kmdrv->seq_num++;
	report->context = (void *)kmdrv->token_interval;
	report->report_id = 3;
	report->report_type = HID_REPORT_TYPE_INPUT;
	report->interface = KBD1_INTF_NUM;
	report->report_length = 1 + ((my_field->report->size - 1) >> 3) + 1; //The answer should be 8 bytes. This is a simplified method.
#if 0
	if (0) {
		int i;
		printk("kbd1_3 report_buf dump = ");
		for (i = 0; i < report->report_length; i++) {
			printk(" %02X", report_buf[i]);
		}
		printk("\n");
	}
#endif
}

static void _hidkm_fill_report_event_ms0(struct kmoip_drv *kmdrv, kmr_target_s *target, struct kmoip_report *report)
{
	__u8 *report_buf = report->report;
	struct hid_field *my_field = NULL;

	kmdrv->ms0_event_pending = 0;

	memset(report_buf, 0, KMOIP_MAX_REPORT_BUF_SIZE);

	//printk("report->type=%x\n", report->type);

	my_field = hidkm_find_field_by_usage(kmdrv->ms0_hiddev, (__u32)(HID_UP_BUTTON|HID_USG09_MIN), HID_INPUT_REPORT);
	if (my_field == NULL) {
		uerr("Can't find my_field? (modifier)\n");
		return;
	}
	hidkm_set_report_field(my_field, report_buf);

	my_field = hidkm_find_field_by_usage(kmdrv->ms0_hiddev, (__u32)(HID_GD_X), HID_INPUT_REPORT);
	if (my_field == NULL) {
		uerr("Can't find my_field (keycode)?\n");
		return;
	}
	hidkm_set_report_field(my_field, report_buf);

	my_field = hidkm_find_field_by_usage(kmdrv->ms0_hiddev, (__u32)(HID_GD_WHEEL), HID_INPUT_REPORT);
	if (my_field == NULL) {
		uerr("Can't find my_field (keycode)?\n");
		return;
	}
	hidkm_set_report_field(my_field, report_buf);

	report->magic_num = KMOIP_MAGIC_NUM;
	report->from = kmdrv->id;
	report->to = target->host_id;
	report->seq_num = kmdrv->seq_num++;
	report->context = (void *)kmdrv->token_interval;
	report->report_id = 0;
	report->report_type = HID_REPORT_TYPE_INPUT;
	report->interface = MS0_INTF_NUM;
	report->report_length = ((my_field->report->size - 1) >> 3) + 1; //The answer should be 4 bytes. This is a simplified method.
#if 0
	if (0) {
		int i;
		printk("ms0 report_buf dump = ");
		for (i = 0; i < report->report_length; i++) {
			printk(" %02X", report_buf[i]);
		}
		printk("\n");
	}
#endif
}

static void _hidkm_fill_report_event_tch0(struct kmoip_drv *kmdrv, kmr_target_s *target, struct kmoip_report *report)
{
	__u8 *report_buf = report->report;
	struct hid_field *my_field = NULL;

	kmdrv->tch0_event_pending = 0;

	memset(report_buf, 0, KMOIP_MAX_REPORT_BUF_SIZE);

	//printk("report->type=%x\n", report->type);

	my_field = hidkm_find_field_by_usage(kmdrv->tch0_hiddev, (__u32)(HID_UP_BUTTON|HID_USG09_MIN), HID_INPUT_REPORT);
	if (my_field == NULL) {
		uerr("Can't find my_field? (tch0 button)\n");
		return;
	}
	hidkm_set_report_field(my_field, report_buf);

	my_field = hidkm_find_field_by_usage(kmdrv->tch0_hiddev, (__u32)(HID_GD_X), HID_INPUT_REPORT);
	if (my_field == NULL) {
		uerr("Can't find my_field (tch0 pointer)?\n");
		return;
	}
	/* This is a special KMoIP roaming hack. Hide cursor of inactive screen. */
	if (kmr_get_cur_target(&kmdrv->roaming) != target) {
		/* Hide the cursor to top-right corner. */
		hidkm_set_field_value_ex(my_field, HID_USG01_X, my_field->logical_maximum);
		hidkm_set_field_value_ex(my_field, HID_USG01_Y, my_field->logical_minimum);
	}
	hidkm_set_report_field(my_field, report_buf);

	my_field = hidkm_find_field_by_usage(kmdrv->tch0_hiddev, (__u32)(HID_GD_WHEEL), HID_INPUT_REPORT);
	if (my_field == NULL) {
		uerr("Can't find my_field (tch0 wheel)?\n");
		return;
	}
	hidkm_set_report_field(my_field, report_buf);

	report->magic_num = KMOIP_MAGIC_NUM;
	report->from = kmdrv->id;
	report->to = target->host_id;
	report->seq_num = kmdrv->seq_num++;
	report->context = (void *)kmdrv->token_interval;
	report->report_id = 0;
	report->report_type = HID_REPORT_TYPE_INPUT;
	report->interface = TCH0_INTF_NUM;
	report->report_length = ((my_field->report->size - 1) >> 3) + 1; //The answer should be 5 bytes. This is a simplified method.
#if 0
	if (0) {
		int i;
		printk("tch0 report_buf dump = ");
		for (i = 0; i < report->report_length; i++) {
			printk(" %02X", report_buf[i]);
		}
		printk("\n");
	}
#endif
}

enum kmoip_report_type {
	REPORT_KBD0 = 0,
	REPORT_KBD1_1,
	REPORT_KBD1_3,
	REPORT_MS0,
	REPORT_TCH0
};

static void hidkm_fill_report_event(struct kmoip_drv *kmdrv, kmr_target_s *target, enum kmoip_report_type rtype)
{
	struct kmoip_report *report;
	unsigned int is_pending = 0;
	static void (*fill)(struct kmoip_drv *, kmr_target_s *target, struct kmoip_report *);

	switch (rtype) {
		case REPORT_KBD0:
			is_pending = kmdrv->kbd0_event_pending;
			fill = _hidkm_fill_report_event_kbd0;
			break;
		case REPORT_KBD1_1:
			is_pending = kmdrv->kbd1_1_event_pending;
			fill = _hidkm_fill_report_event_kbd1_1;
			break;
		case REPORT_KBD1_3:
			is_pending = kmdrv->kbd1_3_event_pending;
			fill = _hidkm_fill_report_event_kbd1_3;
			break;
		case REPORT_MS0:
			is_pending = kmdrv->ms0_event_pending;
			fill = _hidkm_fill_report_event_ms0;
			break;
		case REPORT_TCH0:
			is_pending = kmdrv->tch0_event_pending;
			fill = _hidkm_fill_report_event_tch0;
			break;
		default:
			return;
	}

	if (!is_pending)
		return;

	report = dequeue_report_pool_fifo(kmdrv, 1);
	if (!report) {
		/* Long debug message in urb irq make things worse. */
		printk("M");
		return;
	}

	fill(kmdrv, target, report);
	//BruceTBD. Handle fill() error

	enqueue_report_fifo(kmdrv, target, report, 1);
}

int hidkm_report_event(struct hid_device *hid, struct hid_report *report)
{
	struct kmoip_drv *kmdrv;
	roaming_s *roaming;
	kmr_target_s *target;

	//hidkm_report_event_kbd0(hid, report);
	//hidkm_report_event_ms0(hid, report);

	kmdrv = hid->kmdrv;
	if (unlikely(!kmdrv))
		return 0;

	roaming = &kmdrv->roaming;
	target = kmr_get_cur_target(roaming);

	/* Must be called before fill report event so that we can hide cursor. */
	if (kmr_is_roaming_enabled(roaming) && !kmr_target_is_local(roaming, target))
		kmr_try_roaming(roaming, target);

	hidkm_fill_report_event(kmdrv, target, REPORT_KBD0);
	hidkm_fill_report_event(kmdrv, target, REPORT_KBD1_1);
	hidkm_fill_report_event(kmdrv, target, REPORT_KBD1_3);
	hidkm_fill_report_event(kmdrv, target, REPORT_MS0);
	hidkm_fill_report_event(kmdrv, target, REPORT_TCH0);

	kmoip_kick_upstream_handler(kmdrv, target);

	if (!kmdrv->is_socket_ready)
		return 0;

	return 1; //don't pass to input subsystem
}

//Copied from hid-input.c
static int hidinput_find_field(struct hid_device *hid, unsigned int type, unsigned int code, struct hid_field **field)
{
	struct hid_report *report;
	int i, j;

	list_for_each_entry(report, &hid->report_enum[HID_OUTPUT_REPORT].report_list, list) {
		for (i = 0; i < report->maxfield; i++) {
			*field = report->field[i];
			for (j = 0; j < (*field)->maxusage; j++)
				if ((*field)->usage[j].type == type && (*field)->usage[j].code == code)
					return j;
		}
	}
	return -1;
}
//Copied from hid-input.c
int hidkm_input_event(struct hid_device *hid, struct kmoip_report *report)
{
	struct hid_field *field;
	int offset;
	unsigned int code;
	int value;

	//Code starts from 0x1:NumLock to 0x3:ScrollLock.
	for (code = LED_NUML; code <= LED_SCROLLL; code++) {

		if ((offset = hidinput_find_field(hid, EV_LED, code, &field)) == -1) {
			continue;
		}
		value = (report->report[0] >> code) & 1;
		//uinfo("set code(%d) offset(%d) to value(%d)\n", code, offset, value);
		hid_set_field(field, offset, value);
		hid_submit_report(hid, field->report, USB_DIR_OUT);
	}

	return 0;
}


EXPORT_SYMBOL(hidkm_report_event);
EXPORT_SYMBOL(hidkm_process_event);
EXPORT_SYMBOL(ast_create_hid_class_device);
EXPORT_SYMBOL(hidkm_input_event);

