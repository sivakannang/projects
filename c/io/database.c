#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

const char db_name[] = "db.dat";

struct STUDENT {
	long rollno;
	char name[128];
	char gender;
};

typedef struct STUDENT Student;

bool db_exists() {
	bool is_exists = false;
	FILE *fp = fopen(db_name, "rb");
	if ( fp ) {
		fclose(fp);
		is_exists = true;
		printf("\nDB exists -> %s", db_name);
	} else {
		printf("\nDB not exists -> %s", db_name);
	}
	return is_exists;
}

bool db_create() {
	bool status = false;
	FILE *fp = fopen(db_name, "wb");
	if ( fp ) {
		fclose(fp);
		status = true;
		printf("\nDB created -> %s", db_name);
	} else {
		printf("\nDB creation failed -> %s", db_name);
	}
	return status;
}

Student* db_find(long rollno) {
	
	static Student student;
	int ret = 0;
	
	FILE *fp = fopen(db_name, "rb");
	if ( fp == NULL ) {
		printf("\n%s -> Unable to open db", __func__);
		return NULL;
	}

	memset(&student, 0, sizeof(student));
	while ( 1 ) {
	        ret = fread(&student, sizeof(Student), 1, fp);
		if ( ret <= 0 || rollno == student.rollno )
			break;
		memset(&student, 0, sizeof(student));
	}
	fclose(fp);
	return (rollno == student.rollno) ? &student : NULL;
}

bool db_insert(Student *student) {

	if ( db_find(student->rollno) ) {
		printf("\n duplicate not allowed -> %d", student->rollno);
		return false;
	}

	bool status = false;
	FILE *fp = fopen(db_name, "ab");
	if ( fp ) {
		fwrite(student, sizeof(Student), 1, fp);
		fclose(fp);
		status = true;
		printf("\n %s -> %d ", __func__, student->rollno);
	}
	return status;
}

bool db_delete(long rollno) {

	bool status = false;
	Student student = {0};
	FILE *fp = fopen(db_name, "r+");
	if ( fp == NULL ) {
		printf("\n %s -> %d -> DB open failed", __func__, rollno);
		return false;
	}

	while ( 1 ) {
	        int ret = fread(&student, sizeof(Student), 1, fp);
		if ( ret <= 0 || rollno == student.rollno )
			break;
		memset(&student, 0, sizeof(student));
	}

	if ( rollno == student.rollno ) {
		printf("\n ftell(fp) = %d", ftell(fp));
		fseek(fp, - sizeof(Student), SEEK_CUR);
		printf("\n ftell(fp) = %d", ftell(fp));
		ftruncate( fileno(fp), sizeof(Student));
		status = true;
	}
	fclose(fp);
	return status;

}


int main() {

	if ( !db_exists() )
		db_create();

	Student s1 = {1, "sivakannan", 'M'};
	Student s2 = {2, "sivaganeshan", 'M'};
	Student s3 = {3, "sivakarthik", 'M'};

	db_insert(&s1);
	db_insert(&s2);
	db_insert(&s3);

	db_delete(1);
}
