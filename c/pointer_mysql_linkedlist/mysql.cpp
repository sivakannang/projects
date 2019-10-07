
/****************************** Scope ******************************************

1. mysql setup
2. mysql basic quries
3. Accessing mysql databases through C programs

1. Pointers
2. Double pointers
3. Linked List
4. Dynamic memory allocation
5. Functions


Discussions :

Memory Leak, Dangling pointer
https://en.wikipedia.org/wiki/Memory_leak

Windows   => cl mysql.cpp libmysql.lib -I"C:\Program Files\MySQL\MySQL Connector C 6.1\include"
Linux     => g++ mysql.cpp -lmysqlclient -I"C:\Program Files\MySQL\MySQL Connector C 6.1\include" -o mysql

*******************************************************************************/


/****************************** SQL ********************************************

SQL is a standard language for accessing and manipulating databases.
 * SQL stands for Structured Query Language
 * SQL lets you access and manipulate databases
 * SQL is an ANSI (American National Standards Institute) standard


What Can SQL do?
 * SQL can execute queries against a database
 * SQL can retrieve data from a database
 * SQL can insert records in a database
 * SQL can update records in a database
 * SQL can delete records from a database
 * SQL can create new databases
 * SQL can create new tables in a database
 * SQL can create stored procedures in a database
 * SQL can create views in a database
 * SQL can set permissions on tables, procedures, and views


SQL is a standard - But ...
 * Although SQL is an ANSI (American National Standards Institute) standard, there are different versions of the SQL language.
 * However, to be compliant with the ANSI standard, they all support at least the major commands (such as SELECT, UPDATE, DELETE, INSERT, WHERE) in a similar manner.


Some of The Most Important SQL Commands
 * SELECT - extracts data from a database
 * UPDATE - updates data in a database
 * DELETE - deletes data from a database
 * INSERT INTO - inserts new data into a database
 * CREATE DATABASE - creates a new database
 * ALTER DATABASE - modifies a database
 * CREATE TABLE - creates a new table
 * ALTER TABLE - modifies a table
 * DROP TABLE - deletes a table
 * CREATE INDEX - creates an index (search key)
 * DROP INDEX - deletes an index



Some example of mysql cmds
 * mysql
 * mysql -h127.0.0.1
 * mysql -h127.0.0.1 -uroot
 * mysql -h127.0.0.1 -uroot -proot database
 * mysql -h127.0.0.1 -uroot -proot companies -e "delete table girmiti"

 
Some example of mysql queries
 
 * List databases
   > show databases;
 * Create a database
   > create database 'database_name'
 * Delete a database
   > drop database 'database_name'
 * Select a database
   > use 'database_name'
 * To know the working/current database
   > select database()
 
 * List tables
   > show tables
 * Create a table
   > create table 'table_name'('field1_name' 'data_type', ... , 'fieldn_name' 'data_type')
 * Alter a table
   > alter table 'table_name' add 'column_name' 'datatype'
   > alter table 'table_name' drop 'column_name'
   > alter table 'table_name' modify column 'column_name' 'datatype'
 * Delete a table and table contents
   > drop table 'table_name'
 * Delete contents of table
   > delete / delete * from 'table_name'

 * List record's from a table
   > select *from 'table_name'
 * List particular record/records from a table
   > select 'field_1' from 'table_name'
   > select 'field1', 'field_2' from 'table_name';
 * Insert a record into a table
   > insert into 'table_name' values('value1', 'value2', 'value3',...);
 * Delete a record/record's by condition
   > delete from 'table_name' where 'some_column' = 'some_value'
 * Update/Modify a field values in a record
   > update 'table_name' set 'column_1' = 'value_1', 'column_2'='value_2', ... , 'column_n"='value_n'

 * where , and, or, order by, asc|desc , distinct, limit, in, between, like
   > select *from 'table_name' where 'some_column'='some_value'
   > select *from 'table_name' where 'some_column_1'='some_value_1' and 'some_column_2'='some_value_2'
   > select *from 'table_name' where 'some_column_1'='some_value_1' or  'some_column_2'='some_value_2'
   > select *from 'table_name' order by 'column_name' asc|desc
   > select distinct 'column_name' from 'table_name'
   > select *from 'table_name' limit 3
   > select *from 'table_name' where 'column_name' in('value1', 'value2', ... ')
   > select *from 'table_name' where 'column_name' between 'value1' and 'value2'
   > select *from 'table_name' where 'column_name' like 'pattern'

* SQL Functions
  > select avg('column_name') from 'table_name'
  > select count(*) from 'table_name'
  > select count('column_name') from 'table_name'  => count except NULL values
  > select sum('column_name') from 'table_name'
  > select max('column_name') from 'table_name'
  > select min('column_name') from 'table_name'
  > select ucase('column_name') from 'table_name'
  > select lcase('column_name') from 'table_name'
  > select mid('column_name',start,length) from 'table_name'


*******************************************************************************/


#ifdef _WIN32
//#pragma comment(lib, "libmysql.lib")
#else
#endif

#include <my_global.h>
#include <mysql.h>



#define HOST "192.168.0.6"
#define USER "root"
#define PASS "root"
#define DBNAME "companies"

typedef enum
{
	FAIL,
	SUCCESS
}STATUS;


struct Field
{
	char *name;
	struct Field *next;
};
typedef struct Field FIELD;

struct Record
{
	char **values;
	struct Record *next;
};
typedef struct Record RECORD;

struct Table
{
	char *name;
	short fieldCount;
	FIELD *field;
	RECORD *records;
	struct Table *next;
};
typedef struct Table TABLE; 

struct Database
{
	char *name;
	TABLE *table;
	struct Database *next;
};
typedef struct Database DATABASE;

DATABASE *getDbList(MYSQL *handle);
TABLE    *getTableList(MYSQL *handle, DATABASE *database);
FIELD    *getFieldList(MYSQL *handle, TABLE *table);
RECORD   *getRecordList(MYSQL *handle, TABLE *table);

DATABASE *make_node_db(char *name);
TABLE    *make_node_table(char *name);
FIELD    *make_node_field(char *name);
RECORD   *make_node_record(int num_fields);

DATABASE *list_insert_db(DATABASE *head, DATABASE *newNode);
TABLE    *list_insert_table(TABLE *head, TABLE *newNode);
FIELD    *list_insert_field(FIELD *head, FIELD *newNode);
RECORD   *list_insert_record(RECORD *head, RECORD *newNode);
RECORD   *list_insert_record_value(RECORD *record, char *value, int idx);

void list_display_dbs(DATABASE *head);
void list_display_tables(DATABASE *database);
void list_display_fields(TABLE *table);
void list_display_records(TABLE *table);

void freeDatabase(DATABASE *database);
void freeTable(TABLE *table);
void freeField(FIELD *field);
void freeRecord(RECORD *record, int num_fields);
void releaseMemory(DATABASE *head);

MYSQL *SQL_Connect(const char *database);
void   SQL_Disconnect(MYSQL *handle);
STATUS DB_Create(MYSQL *handle, const char *dbName);
STATUS DB_Drop(MYSQL *handle, const char *dbName);
STATUS Query_Execute(MYSQL *handle, const char *query);


STATUS sqlTest();
STATUS linkedListTest();




MYSQL *SQL_Connect(const char *database)
{
	MYSQL *handle = mysql_init(NULL);
	if ( handle == NULL){
		fprintf(stderr, "\nmysql_init() failed");
		return NULL;
	}

	if( mysql_real_connect(handle, HOST, USER, PASS, database, 0, NULL, 0) == NULL)
	{
		fprintf(stderr, "\nmysql_real_connect() fail : %s", mysql_error(handle));
		mysql_close(handle);
		return NULL;
	}

	return handle;
}

void SQL_Disconnect(MYSQL *handle)
{
	mysql_close(handle);
}

STATUS DB_Create(MYSQL *handle, const char *dbName)
{
	char query[126] = {0};

	sprintf(query, "CREATE DATABASE %s", dbName);

	return Query_Execute(handle, query);
}

STATUS DB_Drop(MYSQL *handle, const char *dbName)
{
	char query[126] = {0};

	sprintf(query, "DROP DATABASE %s", dbName);

	return Query_Execute(handle, query);

}

STATUS Query_Execute(MYSQL *handle, const char *query)
{
	if ( mysql_query(handle, query))
	{
		fprintf(stderr, "\nmysql_query() failed => %s => %s", query, mysql_error(handle));
		return FAIL;
	}

	return SUCCESS;
}





STATUS sqlTest()
{
	STATUS status;
	MYSQL *handle = SQL_Connect(NULL);
	if ( handle == NULL)
		return FAIL;

	DB_Drop(handle, DBNAME);
	
	status = DB_Create(handle, DBNAME);
	if ( status != SUCCESS)
		goto disconnect;
	SQL_Disconnect(handle);

	handle = SQL_Connect(DBNAME);
	if ( handle == NULL)
		return FAIL;

	status = Query_Execute(handle, "create table innoviti(emp_id int, name varchar(48), gender char, age int, team varchar(24))");
	status = Query_Execute(handle, "create table girmiti(emp_id int, name varchar(48), gender char, age int, team varchar(24))");
	if ( status != SUCCESS)
		goto disconnect;

	status = Query_Execute(handle, "insert into innoviti values(101, 'Ganesh', 'M', 25, 'Development')");
	status = Query_Execute(handle, "insert into innoviti values(102, 'Kasi', 'M', 28, 'Development')");
	if ( status != SUCCESS)
		goto disconnect;

	status = Query_Execute(handle, "insert into girmiti values(111, 'Siva', 'M', 27, 'Development')");
	status = Query_Execute(handle, "insert into girmiti values(112, 'Ranjith', 'M', 24, 'Development')");
	if ( status != SUCCESS)
		goto disconnect;


disconnect:
	SQL_Disconnect(handle);

	return status;

}





DATABASE *getDbList(MYSQL *handle)
{
	int num_fields = 0;
	STATUS status;
	DATABASE *head = NULL;

	MYSQL_RES *result = NULL;
	MYSQL_ROW row;

	status = Query_Execute(handle, "show databases");
	if ( status != SUCCESS)
		return NULL;

	result = mysql_store_result(handle);
	if ( result == NULL)
		return NULL;

	num_fields = mysql_num_fields(result);
	while ((row = mysql_fetch_row(result))) 
	{
		DATABASE *node = make_node_db(row[0]);
		head = list_insert_db(head, node);
	}

	mysql_free_result(result);
	
	return head;
}

TABLE *getTableList(MYSQL *handle, DATABASE *database)
{
	int num_fields = 0;
	STATUS status;
	TABLE *head = NULL;

	MYSQL_RES *result = NULL;
	MYSQL_ROW row;
	
	status = Query_Execute(handle, "show tables");
	if ( status != SUCCESS)
		return NULL;

	result = mysql_store_result(handle);
	if ( result == NULL)
		return NULL;

	num_fields = mysql_num_fields(result);
	while ((row = mysql_fetch_row(result))) 
	{
		TABLE *node = make_node_table(row[0]);
		head = list_insert_table(head, node);
	}

	mysql_free_result(result);

	return head;
}

FIELD *getFieldList(MYSQL *handle, TABLE *table)
{
	int num_fields = 0;
	char query[128] = {0};

	STATUS status;
	FIELD *head = NULL;

	MYSQL_RES *result = NULL;
	MYSQL_ROW row;
	
	sprintf(query, "desc %s", table->name);

	status = Query_Execute(handle, query);
	if ( status != SUCCESS)
		return NULL;

	result = mysql_store_result(handle);
	if ( result == NULL)
		return NULL;

	num_fields = mysql_num_fields(result);

	while ((row = mysql_fetch_row(result))) 
	{
		FIELD *node = make_node_field(row[0]);
		head = list_insert_field(head, node);
	}

	mysql_free_result(result);
	
	return head;
}

RECORD *getRecordList(MYSQL *handle, TABLE *table)
{
	int num_fields = 0;
	char query[128] = {0};

	STATUS status;
	RECORD *head = NULL;

	MYSQL_RES *result = NULL;
	MYSQL_ROW row;
	

	sprintf(query, "select *from %s", table->name);

	status = Query_Execute(handle, query);
	if ( status != SUCCESS)
		return NULL;

	result = mysql_store_result(handle);
	if ( result == NULL)
		return NULL;

	num_fields = mysql_num_fields(result);
	table->fieldCount = num_fields;

	while ((row = mysql_fetch_row(result))) 
	{
		RECORD *node = make_node_record(num_fields);

		for(int idx = 0; idx < num_fields; idx++) 
			node = list_insert_record_value(node, row[idx], idx);

		head = list_insert_record(head, node);
	}

	mysql_free_result(result);
	
	return head;
}


DATABASE *make_node_db(char *name)
{
	DATABASE *database = (DATABASE *)calloc(1, sizeof(DATABASE));
	assert(database != NULL);

	database->name = (char *)calloc(1, strlen(name)+1);

	sprintf(database->name, "%s", name);
	
	return database;
}

TABLE *make_node_table(char *name)
{
	TABLE *table = (TABLE *)calloc(1, sizeof(TABLE));
	assert(table != NULL);

	table->name = (char *)calloc(1, strlen(name)+1);

	sprintf(table->name, "%s", name);
	
	return table;
}

FIELD *make_node_field(char *name)
{
	FIELD *field = (FIELD *)calloc(1, sizeof(FIELD));
	assert(field != NULL);

	field->name = (char *)calloc(1, strlen(name)+1);

	sprintf(field->name, "%s", name);
	
	return field;
}

RECORD *make_node_record(int num_fields)
{
	RECORD *record = (RECORD *)calloc(1, sizeof(RECORD));
	assert(record != NULL);

	record->values = (char **)calloc(num_fields, sizeof(char *));
	
	return record;
}


DATABASE *list_insert_db(DATABASE *head, DATABASE *newNode)
{
	DATABASE *ptr = head;

	if ( ptr == NULL)
		return newNode;

	while ( ptr->next)
		ptr = ptr->next;

	ptr->next = newNode;

	return head;

}

TABLE *list_insert_table(TABLE *head, TABLE *newNode)
{
	TABLE *ptr = head;

	if ( ptr == NULL)
		return newNode;

	while ( ptr->next)
		ptr = ptr->next;

	ptr->next = newNode;

	return head;

}

FIELD *list_insert_field(FIELD *head, FIELD *newNode)
{
	FIELD *ptr = head;

	if ( ptr == NULL)
		return newNode;

	while ( ptr->next)
		ptr = ptr->next;

	ptr->next = newNode;

	return head;

}

RECORD *list_insert_record(RECORD *head, RECORD *newNode)
{
	RECORD *ptr = head;

	if ( ptr == NULL)
		return newNode;

	while ( ptr->next)
		ptr = ptr->next;

	ptr->next = newNode;

	return head;

}

RECORD *list_insert_record_value(RECORD *record, char *value, int idx)
{
	if ( value == NULL){
		record->values[idx] = NULL;
		return record;
	}
	
	record->values[idx] = (char *)calloc(1, strlen(value)+1);

	sprintf(record->values[idx], "%s", value);

	return record;
}


void list_display_dbs(DATABASE *head)
{
	DATABASE *database = head;
	
	while(database)
	{
		list_display_tables(database);
		database = database->next;
	}
}

void list_display_tables(DATABASE *database)
{
	printf("\n\n--------------------- Displaying databse => %s---------\n", database->name);

	TABLE *table = database->table;
	while(table)
	{
		list_display_fields(table);
		list_display_records(table);
		table = table->next;
	}
}


void list_display_fields(TABLE *table)
{
	FIELD *field = table->field;
	printf("\n\n-----------------Displaying table -> %s---------\n", table->name);
	while(field)
	{
		printf("%s\t", field->name);
		field = field->next;
	}
}


void list_display_records(TABLE *table)
{
	RECORD *record = table->records;
	printf("\n\n");
	while(record)
	{
		printf("\n");
		for ( int i = 0; i < table->fieldCount; i++)
			printf("%s\t", record->values[i] );
		record = record->next;
	}
}


STATUS linkedListTest()
{
	MYSQL *handle = NULL;
	DATABASE *head = NULL;
	DATABASE *database = NULL;
	
	handle = SQL_Connect(NULL);
	if ( handle == NULL)
		return FAIL;

	head = getDbList(handle);
	database = head;
	SQL_Disconnect(handle);

	while(database)
	{
		handle = SQL_Connect(database->name);
		database->table = getTableList(handle, database);
		TABLE *table = database->table;
		printf("\n -----------------------Processing database => %s ----------------", database->name);
		while(table )
		{
			printf("\n\t Processing table -> %s", table->name);
			table->field = getFieldList(handle, table);
			table->records = getRecordList(handle, table);
			table = table->next;
		}
		database = database->next;
		SQL_Disconnect(handle);
	}

	list_display_dbs(head);

	releaseMemory(head);

	return SUCCESS;
}

void releaseMemory(DATABASE *head)
{
	DATABASE *database = head;

	while(database)
	{
		DATABASE *tmp = database->next;
		freeDatabase(database);
		database = tmp;
	}
}


void freeDatabase(DATABASE *database)
{
	TABLE *table = database->table;

	printf("\n\n\n ----------------- Freeing Database => %s -----------------------", database->name);
	free(database->name), database->name = NULL;

	while(table)
	{
		TABLE *tmp = table->next;
		freeTable(table);
		table = tmp;
	}

	free(database), database = NULL;
}


void freeTable(TABLE *table)
{
	RECORD *record = table->records;
	FIELD *field = table->field;

	printf("\n\tFreeing table -> %s", table->name);
	free(table->name), table->name = NULL;
	
	while(field)
	{
		FIELD *tmp = field->next;
		freeField(field);
		field = tmp;
	}

	while(record)
	{
		RECORD *tmp = record->next;
		freeRecord(record, table->fieldCount);
		record = tmp;
	}

	free(table), table = NULL;
	
}

void freeField(FIELD *field)
{
	free(field->name), field->name = NULL;
	free(field), field = NULL;
}

void freeRecord(RECORD *record, int num_fields)
{
	for ( int i = 0; i < num_fields; i++)
		if ( record->values[i] )
			free(record->values[i]), record->values[i] = NULL;

	free(record->values), record->values = NULL;

	free(record), record = NULL;
}


int main()
{
	//sqlTest();
	linkedListTest();
	return 0;
}



