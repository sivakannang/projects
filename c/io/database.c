// gcc database.c -o database

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

const char db_name[] = "db.dat";
const char db_temp[] = "tmp.dat";

struct STUDENT {
	long rollno;
	char name[128];
	char gender;
};

typedef struct STUDENT Student;

FILE *db_open(const char *filename, const char *mode)
{
	FILE *fp = fopen(filename, mode);
	if ( fp == NULL )
	{
		printf("\n DB Open failed - filename = %s , mode = %s", filename, mode);
		return NULL;
	}
	printf("\n DB open success - filename = %s , mode = %s", filename, mode);
	return fp;
}

void db_close(FILE *fp)
{
	if ( fp )
	{
		fclose(fp);
		printf("\n DB Close success");
	}
}

bool db_exists()
{
	FILE *fp = db_open(db_name, "r");
	if ( fp == NULL )
	{
		return false;
	}
	db_close(fp);

	return true;
}

bool db_create()
{
	FILE *fp = fopen(db_name, "wb");
	if ( fp )
	{
		fclose(fp);
		printf("\nDB created -> %s", db_name);
		return true;
	}
	printf("\nDB creation failed -> %s", db_name);
	
	return false;
}

bool db_list()
{
	int ret = 0;
	printf("\n ........... DB List begins .......... ");
	FILE *fp = db_open(db_name, "r");
	if ( fp == NULL )
	{
		return false;
	}

	Student student;
	while ( 1 )
	{
		memset(&student, 0, sizeof(student));
		ret = fread(&student, sizeof(student), 1, fp);
		if ( ret != 1 )
		{
			break;
		}
		printf("\n rollno = %ld , name = %s , gender = %c", student.rollno, student.name, student.gender);
	}
	fclose(fp);
	printf("\n ........... DB List ends .......... ");
	return true;
}

Student* db_find(long rollno)
{	
	static Student student;
	int ret = 0;
	
	FILE *fp = fopen(db_name, "rb");
	if ( fp == NULL )
	{
		printf("\n DB Open failed - %s", db_name);
		return NULL;
	}

	// Check fread first: comparing after end of file could match leftover or zeroed data
	while ( (ret = fread(&student, sizeof(Student), 1, fp)) == 1 && rollno != student.rollno )
		;

	fclose(fp);

	return (ret == 1) ? &student : NULL;
}

bool db_insert(Student *student) {

	if ( db_find(student->rollno) )
	{
		printf("\n duplicate not allowed -> %ld", student->rollno);
		return false;
	}

	FILE *fp = fopen(db_name, "ab");
	if ( fp == NULL )
	{
		printf("\n DB Open failed - %s", db_name);
		return false;
	}

	// fclose flushes the buffered data, so a full disk can make it fail too
	if ( fwrite(student, sizeof(Student), 1, fp) != 1 || fclose(fp) != 0 )
	{
		printf("\n DB write failed - %s", strerror(errno));
		return false;
	}

	printf("\n %s -> rec inserted for rollno %ld ", __func__, student->rollno);

	return true;
}

bool db_delete(long rollno)
{
	bool found = false;
	bool writeFailed = false;
	Student student;

	FILE *dbFP  = db_open(db_name, "r");
	if ( dbFP == NULL )
	{
		return false;
	}

	FILE *tmpFP = db_open(db_temp, "w");
	if ( tmpFP == NULL )
	{
		db_close(dbFP);
		return false;
	}

	while ( fread(&student, sizeof(Student), 1, dbFP) == 1 )
	{
		if ( rollno == student.rollno )
		{
			found = true;
			continue;
		}
		if ( fwrite(&student, sizeof(student), 1, tmpFP) != 1 )
		{
			writeFailed = true;
			break;
		}
	}

	if ( ferror(dbFP) )
		writeFailed = true;

	db_close(dbFP);

	if ( fclose(tmpFP) != 0 )
		writeFailed = true;

	// rename replaces the database in one step, so never do it with an incomplete copy
	if ( writeFailed )
	{
		printf("\n db delete failed, database left unchanged");
		remove(db_temp);
		return false;
	}

	if ( rename(db_temp, db_name) != 0 )
	{
		printf("\n db rename failed , errno = %d , errstr = %s", errno, strerror(errno));
		remove(db_temp);
		return false;
	}

	return found;
}


int main() {

	if ( !db_exists() && !db_create() )
		return 1;

	Student s1 = {1, "sivakannan", 'M'};
	Student s2 = {2, "sivaganeshan", 'M'};
	Student s3 = {3, "sivakarthikeyan", 'M'};

	db_insert(&s1);
	db_insert(&s2);
	db_insert(&s3);

	db_list();

	db_delete(1);

	db_list();

	printf("\n");

	return 0;
}
