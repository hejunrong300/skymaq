#include "linklist.h"
#include <string.h>
/*===============================================
*   文件名称：linklist.c
*   创 建 者：hejr
*   创建日期：2022年07月28日
*   描    述：
================================================*/

#include "linklist.h"
#include <stdio.h>
#include <stdlib.h>

// 1.创建头结点；
LinkList *Create_Linklist()
{
	LinkList *head = (LinkList *)malloc(sizeof(LinkList));
	if (NULL == head)
	{
		printf("malloc error\n");
		return NULL;
	}
	memset(&head->data, 0, sizeof(data_t));
	head->tail = NULL;
	head->next = head->tail;
	printf("linklist create success.\n");
	return head;
}

// 2.判空
int Linklist_Empty(LinkList *link)
{
	if (link->next == NULL)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

// 3.计算表长
int Linklist_Length(LinkList *link)
{
	int i = 0;
	while (link->next != NULL)
	{
		link = link->next;
		i++;
	}
	return i;
}

int Linklist_Insert(LinkList *list, data_t data)
{
	LinkList *p = (LinkList *)malloc(sizeof(LinkList));
	if (NULL == p)
	{
		printf("malloc error\n");
		return -1;
	}
	memcpy(&p->data, &data, sizeof(data_t));
	if (list->next == NULL)
	{
		list->next = p;
	}
	else
	{
		list->tail->next = p;
	}
	list->tail = p;
	p->next = NULL;

	return 0;
}

int Linklist_pop(LinkList *list, data_t *pdata)
{
	if (Linklist_Empty(list))
	{
		printf("linklist is empty.\n");
		return -1;
	}
	LinkList *p = list->next;
	if (NULL != p)
	{
		memcpy(pdata, &p->data, sizeof(data_t));
		LinkList *q = p;
		list->next = p->next;
		q->next = NULL;
		free(q);
	}

	return 0;
}

// 4.从表头插入节点（头插法/尾插法）
void Linklist_Insert_Head(LinkList *link, data_t data)
{
	LinkList *p = (LinkList *)malloc(sizeof(LinkList));
	if (NULL == p)
	{
		printf("malloc error\n");
		return;
	}
	memcpy(&p->data, &data, sizeof(data_t));

	p->next = link->next;
	link->next = p;
	return;
}

void Linklist_Insert_Front(LinkList *link, data_t data) { return; }

// 5.按位置插入数据
void Linklist_Insert_Pos(LinkList *link, int pos, data_t data)
{
	int i = Linklist_Length(link);
	if (pos < 0 || pos > i + 1)
	{
		printf("pos error\n");
		return;
	}
	for (i = 0; i < pos; i++)
	{
		link = link->next;
	}

	LinkList *p = (LinkList *)malloc(sizeof(LinkList));
	memcpy(&p->data, &data, sizeof(data_t));
	p->next = link->next;
	link->next = p;

	return;
}

// 6.按位置删除数据
void Linklist_Delete_Pos(LinkList *link, int pos)
{
	if (Linklist_Empty(link))
	{
		printf("linklist is empty.\n");
		return;
	}

	int i = Linklist_Length(link);
	if (pos < 0 || pos > i + 1)
	{
		printf("pos error\n");
		return;
	}
	for (i = 0; i < pos; i++)
	{
		link = link->next;
	}
	LinkList *p = link->next;

	link->next = p->next;
	free(p);
	p = NULL;

	return;
}

// 7.按数据删除
void Linklist_Delete_Data(LinkList *link, data_t data)
{
	if (Linklist_Empty(link))
	{
		printf("linklist is empty.\n");
		return;
	}
	int i = Linklist_Find_Data(link, data);

	if (i == -1)
		return;

	Linklist_Delete_Pos(link, i);
	return;
}

// 8.按位置查找元素
int Linklist_Find_Pos(LinkList *link, int pos)
{
	int i = Linklist_Length(link);
	if (pos < 0 || pos > i + 1)
	{
		printf("pos error\n");
		return -1;
	}
	for (i = 0; i < pos + 1; i++)
	{
		link = link->next;
	}

	printf("\nlink->data[%d]=%d\n", pos, link->data);
	return i;
}

// 9.按数据查找元素
int Linklist_Find_Data(LinkList *link, data_t data)
{
	int i = -1, j = Linklist_Length(link);

	return i;
}

// 10.按值修改元素
void Linklist_Change_Data(LinkList *link, data_t data, data_t new_data)
{
	int i = Linklist_Find_Data(link, data), j;
	for (j = 0; j <= i; j++)
	{
		link = link->next;
	}
	link->data = new_data;
	return;
}

// 11.清空链表
void Linklist_Setnull(LinkList *link)
{
	LinkList *p = link->next;
	while (p->next != NULL)
	{
		// printf("\np->data=%d\n",p->data);
		Linklist_Delete_Pos(link, 0);
		p = link->next;
	}
	printf("\np->data=%d\n", p->data);
	Linklist_Delete_Pos(link, 0);

	link->next = NULL;
	printf("linklist is cleared.\n");
	return;
}

// 12.删除链表
void Linklist_free(LinkList *link)
{
	if (link->next != NULL)
	{
		Linklist_Setnull(link);
	}
	free(link);
	link = NULL;
	return;
}
// 13.显示链表
void Linklist_Show(LinkList *link)
{
	while (link->next != NULL)
	{
		link = link->next;
		printf("%4d", link->data);
	}
	puts("");
}

LinkList *Order_Linklist(LinkList *link)
{
	LinkList *head = (LinkList *)malloc(sizeof(LinkList));
	if (NULL == head)
	{
		printf("malloc error\n");
		return NULL;
	}

	memset(&head->data, 0, sizeof(data_t));
	head->next = NULL;

	while (link->next != NULL)
	{
		link = link->next;
		Linklist_Insert_Head(head, link->data);
	}

	// Linklist_Show(head);

	return head;
}

void Order_Linklist_1(LinkList *link)
{
	LinkList *head = link->next->next; // 指向第二个
	link->next->next = NULL;		   // 第一个作为结尾，指向NULL

	while (head->next != NULL)
	{
		Linklist_Insert_Head(link, head->data);
		head = head->next;
	}

	Linklist_Insert_Head(link, head->data);

	// Linklist_Show(link);

	return;
}

void Order_Linklist_2(LinkList *link)
{
	LinkList *cur, *pre, *next;
	cur = link->next; // cur指向第一个
	next = cur->next; // next指向第二个
	cur->next = NULL; // 最后一个指向空
	pre = cur;		  // pre指向前一个
	cur = next;		  // cur指向当前

	while (cur->next != NULL)
	{
		next = cur->next; // next指向前一个
		cur->next = pre;  // 链接后一个
		pre = cur;		  // pre前移，为下次链接做准备
		cur = next;		  // cur指向下一次要更改连接的目标
	}
	cur->next = pre;  // 最后一个链接上之前的
	link->next = cur; // 再把头指针指向更改后的第一个元素
}
