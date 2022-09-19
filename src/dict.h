
typedef struct dict dict;

// 字段具体的数据类型
typedef struct dictType{

} dictType;

// dict - hash表结构体
struct dict{

};

/* API */
dict *dictCreate(dictType *type); // 创建hash表