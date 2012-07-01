#include "llist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Pair {
	char name[1024];
	char value[1024];
};

class Tag {
private:
	char tagName[1024];
	LList<Pair*> dataPairs;

	char data[1024];
	
	Tag* parent;
	LList<Tag*>* children;
	int nChildren;
public:
	Tag(char* line, Tag* parent);
	Tag(char* name);
	void addChild(Tag* tag);
	void dropChildren() {delete children; children = new LList<Tag*>();}
	char* getData() {return data;}
	void setData(char* data) {strcpy(this->data, data);}
	char* getName() {return tagName;}
	Tag* getParent() {return parent;}
	void setParent(Tag* parent) {this->parent = parent;}
	Tag* getElementByName(char* name, int nth = 0);
	char* getParameterByName(char* name);
	int getElementCountByName(char* name);
	LList<Tag*>* getChildren() {return children;}
	void printChildren(int indent = 0);
	void print(int indent = 0);
};

void printPair(Pair* pair)
{
	printf("<PAIR> %s->%s\n", pair->name, pair->value);
}

Tag::Tag(char* name)
{
	strcpy(this->tagName, name);
}

Tag::Tag(char* line, Tag* parent)
{
	//Process line
	char* next;
	char* nexts;
	char* s;
	char* v1; 
	char* v2;
	children = new LList<Tag*>();

	char dpair[1024];

	this->parent = parent;

	s = strtok_s(line, " ", &next);
	if(s!=NULL)
		strcpy(this->tagName, s);
	if(next==NULL)
		return;

	do
	{
		Pair *pair = (Pair*)malloc(sizeof(Pair));
		strcpy(dpair, next);
		v1 = strtok_s(dpair, "=", &nexts);
		if(v1!=NULL && v1[0]!='/')
		{
			v2 = nexts;
			strcpy(pair->name, v1);
			if(v2!=NULL)
			{
				char* p = v2;
				int i = 0;

				while(*p!='\"' && *p!=0)
					p++;

				p++;

				while(*p!='\"' && *p!=0)
				{
					pair->value[i++] = *p;
					p++;
				}

				pair->value[i] = 0;

				//strcpy(pair->value, v2);
			}
			else
				strcpy(pair->value, "");
		}
		else
		{
			strcpy(pair->name, "");
			strcpy(pair->value, "");
			break;
		}
		this->dataPairs.addToEnd(pair);
		//printf("%s: %s->%s\n", tagName, pair->name, pair->value);
	}while(s = strtok_s(NULL, " ", &next));
}

void Tag::addChild(Tag* tag)
{
	children->addToEnd(tag);
	nChildren++;
}

Tag* Tag::getElementByName(char* name, int nth)
{
	Tag* t;
	int n=0;

	children->moveCursor(HEAD);
	while(t = children->stepForward())
	{
		if(strcmpi(t->getName(), name)==0)
		{
			if(nth==n)
				return t;

			n++;
		}
	}

	return NULL;
}

char* Tag::getParameterByName(char* name)
{
	Pair* p;

	dataPairs.moveCursor(HEAD);
	while(p = dataPairs.stepForward())
	{
		if(strcmpi(p->name, name)==0)
			return p->value;
	}

	return NULL;
}

int Tag::getElementCountByName(char* name)
{
	Tag* t;
	int n = 0;

	children->moveCursor(HEAD);

	while(t = children->stepForward())
	{
		if(strcmpi(t->getName(), name)==0)
			n++;
	}
		
	return n;
}

void Tag::print(int indent)
{
	char format[1024];
	int i;

	for(int i=0; i<indent; i++)
		printf("\t");
	printf("<TAG> %s:%s\n", this->tagName, this->data);

	for(i=0; i<indent+1; i++)
		format[i] = '\t';

	format[i] = 0;

	dataPairs.print(format, printPair);
}

void Tag::printChildren(int indent)
{
	Tag* t;

	children->moveCursor(HEAD);
	while(t = children->stepForward())
	{
		t->print(indent+1);
		t->printChildren(indent+1);
	}
}

class XML_Document {
private:
	LList<Tag*> parents;
	LList<Tag*> opened;
	int nParents;
public:
	XML_Document(char* xml_file);
	void print();
	void printUnclosed();
	Tag* getElementByName(char* name, int nth = 0);
	int getElementCountByName(char* name);
};

bool tag_equals(Tag* tag1, Tag* tag2)
{
	if(strcmpi(tag1->getName(), tag2->getName())==0)
		return true;
	else
		return false;
}

void print_element(Tag* tag)
{
	printf("Unclosed: %s\n", tag->getName());
}


XML_Document::XML_Document(char* xml_file)
{
	FILE *fp = fopen(xml_file, "r");
	char* buf;
	char temp[1];
	int size = 0;
	char lastFound = ' ';
	Tag* current;

	char* tag;
	int ti = 0;

	char* data;
	int di = 0;
	
	tag = new char[1024];
	data = new char[1024];
	current = NULL;
		
	while(fread(temp, 1, 1, fp))
		size++;

	buf = new char[size];

	fseek(fp, 0, SEEK_SET);

	fread(buf, size, 1, fp);

	for(int i=0; i<size; i++)
	{
		if(buf[i]=='\n')
			continue;

		if(lastFound=='>')
		{
			if(buf[i]=='<')
			{
				lastFound = '<';
				ti = 0;
				data[di] = 0;
				tag = new char[1024];
				current->setData(data);
				continue;
			}
			data[di++] = buf[i];
		}
		else if(lastFound=='<')
		{
			if(buf[i]=='>')
			{
				lastFound = '>';
				di = 0;
				tag[ti] = 0;
				data = new char[1024];
				Tag* temp = new Tag(tag+1);
				if(tag[0]=='/')
				{
					if(opened.find(temp, tag_equals))
					{
						opened.removeAtCursor();
						current = current->getParent();
					}
				}
				else
				{
					if(current==NULL)
					{
						current = new Tag(tag, NULL);
						parents.addToEnd(current);
					}
					else
					{
						Tag* old = current;
						current = new Tag(tag, old);
						old->addChild(current);
					}
					opened.addToEnd(current);
				}
				delete temp;
				continue;
			}
			tag[ti++] = buf[i];
		}
		else
		{
			if(buf[i]=='<')
				lastFound = '<';
		}
	}

	this->printUnclosed();

	Tag* t1;
	Tag* t2;
	LList<Tag*>* lt;
	opened.moveCursor(HEAD);

	while(t1 = opened.stepForward())
	{
		lt = t1->getChildren();
		lt->moveCursor(HEAD);
		while(t2 = lt->stepForward())
		{
			t2->setParent(t1->getParent());
			t1->getParent()->addChild(t2);
		}
		t1->dropChildren();
	}
}

Tag* XML_Document::getElementByName(char* name, int nth)
{
	Tag* t;
	int n = 0;

	parents.moveCursor(HEAD);

	while(t = parents.stepForward())
	{
		if(strcmpi(t->getName(), name)==0)
		{
			if(n==nth)
				return t;

			n++;
		}
	}

	return NULL;
}

int XML_Document::getElementCountByName(char* name)
{
	Tag* t;
	int n = 0;

	parents.moveCursor(HEAD);

	while(t = parents.stepForward())
	{
		if(strcmpi(t->getName(), name)==0)
			n++;
	}
		
	return n;
}

void XML_Document::print()
{
	Tag* t;
	LList<Tag*>* children;
	int indent = 0;

	parents.moveCursor(HEAD);
	while(t = parents.stepForward())
	{
		t->print(indent+1);
		t->printChildren(indent+1);
	}
}

void XML_Document::printUnclosed()
{
	opened.print("", print_element);
}