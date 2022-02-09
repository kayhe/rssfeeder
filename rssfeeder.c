#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

static xmlDocPtr doc;

bool createFeed(char *title, char *description, char *link);
bool addItem(char *title, char *description, char *link, bool generateGUID);
void cleanup();

int main(int argc, char **argv) {
  char *filename = NULL;
  char *title = NULL;
  char *description = NULL;
  char *link = NULL;

  enum operation { NONE, ADD, CREATE };
  enum operation op = NONE;

  bool generateGUID = false;
  int c;

  while ((c = getopt(argc, argv, "acht:l:d:g")) != -1)
    switch (c) {
    case 'a':
      // Add item to feed
      op = ADD;
      break;
    case 'c':
      // Create feed
      op = CREATE;
      break;
    case 't':
      // Feed/Item title
      title = optarg;
      break;
    case 'l':
      // Feed/Item link
      link = optarg;
      break;
    case 'd':
      // Feed/Item description
      description = optarg;
      break;
    case 'g':
      // Generate GUID for item
      generateGUID = true;
      break;
    case 'h':
    case '?':
      // Usage
      printf("Usage:\n");
      printf("-a Add item to feed\n");
      printf("-c Create item to feed\n");
      printf("-t Feed/Item title\n");
      printf("-l Feed/Item link\n");
      printf("-d Feed/Item description\n");
      printf("-g Generate GUID for item\n");
      return EXIT_SUCCESS;
    default:
      return EXIT_FAILURE;
    }

  if (op == NONE) {
    return EXIT_SUCCESS;
  }

  if (optind >= argc) {
    fprintf(stderr, "no filename given\n");
    return EXIT_FAILURE;
  }

  filename = argv[optind];

  int ret = EXIT_SUCCESS;

  if (op == CREATE) {
    FILE *file;
    if ((file = fopen(filename, "r")) != NULL) {
      fprintf(stderr, "File already exists, not overwriting it\n");
      fclose(file);
      return EXIT_SUCCESS;
    }
    if (!createFeed(title, description, link)) {
      ret = EXIT_FAILURE;
    }
  } else if (op == ADD) {
    doc = xmlParseFile(filename);
    if (doc == NULL) {
      fprintf(stderr, "Could not parse file %s\n", filename);
      return EXIT_FAILURE;
    }
    if (!addItem(title, description, link, generateGUID)) {
      ret = EXIT_FAILURE;
    }
  }

  xmlKeepBlanksDefault(0);
  if (ret != EXIT_FAILURE)
    ret = (xmlSaveFormatFileEnc(filename, doc, "utf-8", 1) == -1)
              ? EXIT_FAILURE
              : EXIT_SUCCESS;
  cleanup();
  return ret;
}

void cleanup() {
  xmlFreeDoc(doc);
  xmlCleanupParser();
}

bool createFeed(char *title, char *description, char *link) {
  if (title == NULL || link == NULL) {
    fprintf(stderr, "Cannot create feed, missing arguments\n");
    return false;
  }
  if (description == NULL) {
    description = "";
  }

  xmlNodePtr root;
  xmlNodePtr channelNode;

  doc = xmlNewDoc((const xmlChar *)"1.0");
  root = xmlNewNode(NULL, (const xmlChar *)"rss");
  xmlNewProp(root, (const xmlChar *)"version", (const xmlChar *)"2.0");
  xmlDocSetRootElement(doc, root);
  channelNode =
      xmlNewChild(root, NULL, (const xmlChar *)"channel", (const xmlChar *)"");
  xmlNewChild(channelNode, NULL, (const xmlChar *)"title",
              (const xmlChar *)title);
  xmlNewChild(channelNode, NULL, (const xmlChar *)"description",
              (const xmlChar *)description);
  xmlNewChild(channelNode, NULL, (const xmlChar *)"link",
              (const xmlChar *)link);
  return true;
}

bool addItem(char *title, char *description, char *link, bool generateGUID) {
  xmlNodePtr root = xmlDocGetRootElement(doc);
  if (root == NULL) {
    fprintf(stderr, "empty document\n");
    return false;
  }
  if (xmlStrcmp(root->name, (const xmlChar *)"rss")) {
    fprintf(stderr, "File is not an RSS feed\n");
    return false;
  }
  xmlNodePtr channel = root->children;
  while (channel != NULL) {
    if (!xmlStrcmp(channel->name, (const xmlChar *)"channel")) {
      break;
    }
    channel = channel->next;
  }
  if (channel == NULL) {
    fprintf(stderr, "RSS feed does not contain a channel\n");
    return false;
  }

  char pubDate[50];
  const time_t t = time(NULL);
  const struct tm dateTime = *localtime(&t);
  if (strftime(pubDate, sizeof(pubDate), "%a, %d %b %Y %H:%M:%S %Z",
               &dateTime) == 0) {
    fprintf(stderr, "Can't get current time\n");
    return false;
  }

  xmlNodePtr cur = channel->xmlChildrenNode;
  while (cur != NULL) {
    if (!xmlStrcmp(cur->name, (const xmlChar *)"item") || cur->next == NULL) {
      xmlNodePtr newItem = xmlNewNode(NULL, (const xmlChar *)"item");
      xmlNewChild(newItem, NULL, (const xmlChar *)"title",
                  (const xmlChar *)title);
      if (description != NULL) {
        xmlNewChild(newItem, NULL, (const xmlChar *)"description",
                    (const xmlChar *)description);
      }
      xmlNewChild(newItem, NULL, (const xmlChar *)"link",
                  (const xmlChar *)link);
      if (generateGUID) {
        xmlNodePtr guid = xmlNewChild(newItem, NULL, (const xmlChar *)"guid",
                                      (const xmlChar *)link);
        xmlNewProp(guid, (const xmlChar *)"isPermaLink",
                   (const xmlChar *)"true");
      }
      xmlNewChild(newItem, NULL, (const xmlChar *)"pubDate",
                  (const xmlChar *)pubDate);
      xmlAddPrevSibling(cur, newItem);
      break;
    }
    cur = cur->next;
  }

  return true;
}
