#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <math.h>
#include <string.h>

// 定义用于计算两个经纬度点之间距离的函数
double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    double radianLat1 = lat1 * M_PI / 180.0;
    double radianLon1 = lon1 * M_PI / 180.0;
    double radianLat2 = lat2 * M_PI / 180.0;
    double radianLon2 = lon2 * M_PI / 180.0;
    
    double deltaLat = radianLat2 - radianLat1;
    double deltaLon = radianLon2 - radianLon1;
    
    double a = sin(deltaLat/2.0) * sin(deltaLat/2.0) + cos(radianLat1) * cos(radianLat2) * sin(deltaLon/2.0) * sin(deltaLon/2.0);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    
    // 地球半径（单位：千米）
    double radius = 6371.0;
    
    return radius * c;
}

int main() {
    xmlDocPtr doc;
    xmlNodePtr rootNode, currentNode, ndNode;
    
    // 1. 初始化libxml2库
    LIBXML_TEST_VERSION;

    // 2. 打开XML文件并解析
    doc = xmlReadFile("map.xml", NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse the XML file.\n");
        return 1;
    }

    // 3. 遍历所有way节点
    rootNode = xmlDocGetRootElement(doc);
    currentNode = rootNode->xmlChildrenNode;



    while (currentNode != NULL) {
        if (xmlStrcmp(currentNode->name, (const xmlChar *)"way") == 0) {
            xmlChar *wayIdValue = xmlGetProp(currentNode, (const xmlChar *)"id");
            printf("Way ID: %s\n", wayIdValue);

            char prevNodeId[30] = ""; // 用于存储前一个节点的ID
            double prevLat = 0.0;
            double prevLon = 0.0;
            
            // 在way节点下查找匹配的nd节点
            ndNode = currentNode->xmlChildrenNode;
            while (ndNode != NULL) {
                if (xmlStrcmp(ndNode->name, (const xmlChar *)"nd") == 0) {
                    xmlChar *refValue = xmlGetProp(ndNode, (const xmlChar *)"ref");
                    
                    // 在osm节点下查找匹配的名称为node的子节点
                    xmlNodePtr nodeNode = rootNode->xmlChildrenNode;
                    while (nodeNode != NULL) {
                        if (xmlStrcmp(nodeNode->name, (const xmlChar *)"node") == 0) {
                            xmlChar *nodeIdValue = xmlGetProp(nodeNode, (const xmlChar *)"id");
                            xmlChar *latValue = xmlGetProp(nodeNode, (const xmlChar *)"lat");
                            xmlChar *lonValue = xmlGetProp(nodeNode, (const xmlChar *)"lon");
                            
                            if (nodeIdValue != NULL && latValue != NULL && lonValue != NULL) {
                                if (xmlStrcmp(refValue, nodeIdValue) == 0) {
                                    printf("Node ID: %s, Latitude: %s, Longitude: %s\n",
                                           nodeIdValue, latValue, lonValue);

                                    // 计算节点之间的距离
                                    double lat, lon;
                                    sscanf((char *)latValue, "%lf", &lat);
                                    sscanf((char *)lonValue, "%lf", &lon);

                                    if (strlen(prevNodeId) > 0) {
                                        double distance = calculateDistance(prevLat, prevLon, lat, lon);
                                        printf("Distance between Node ID: %s and Node ID: %s: %.2lf km\n", prevNodeId, nodeIdValue, distance);
                                    }

                                    strcpy(prevNodeId, (char *)nodeIdValue);
                                    prevLat = lat;
                                    prevLon = lon;
                                }
                            }

                            xmlFree(nodeIdValue);
                            xmlFree(latValue);
                            xmlFree(lonValue);
                        }
                        nodeNode = nodeNode->next;
                    }
                    xmlFree(refValue);
                }
                ndNode = ndNode->next;
            }
            
            xmlFree(wayIdValue);
        }
        currentNode = currentNode->next;
    }

    // 4. 清理资源
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    return 0;
}

