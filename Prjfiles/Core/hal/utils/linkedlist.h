//==========================================
//
//	   ZapperOS - Linked List Utility
//
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

template <typename T>
class LinkedList
{
	public:
		struct Node
		{
			T value;
			Node* prevNode;
			Node* nextNode;
		};
		
	public:
		Node* head;
		Node* tail;
		uint32_t length;
		
	public:
		LinkedList();
		void add(T item);
		void remove(Node* node);
};

template <typename T>
LinkedList<T>::LinkedList()
{
	head = NULL;
	tail = NULL;
	length = 0;
}

template <typename T>
void LinkedList<T>::add(T item)
{
	LinkedList<T>::Node* newNode = new LinkedList<T>::Node();
	newNode->value = item;
	newNode->prevNode = tail;
	newNode->nextNode = NULL;
	
	if ((head == NULL) && (tail == NULL))
	{
		head = newNode;
		tail = newNode;		
	}
	else
	{
		tail->nextNode = newNode;
		tail = newNode;
	}
	
	length++;
}

template <typename T>
void LinkedList<T>::remove(Node* node)
{
	node->nextNode->prevNode = node->prevNode;
	node->prevNode->nextNode = node->nextNode;
	
	delete node;

	length--;
}