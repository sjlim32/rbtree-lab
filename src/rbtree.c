#include "rbtree.h"

#include <stdlib.h>
#include <stdio.h>

rbtree *new_rbtree(void) {
  // 새로운 rbtree를 생성할 때 마다, size가 동적으로 1씩 증가
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  
  if (p != NULL) {
    p->nil = (node_t*)calloc(1, sizeof(node_t));
    p->nil->color = RBTREE_BLACK;
    p->root = p->nil;
    
    return p;
  }
  else {
    return -1;
  }

}

void delete_node(rbtree *t, node_t *cur) {
  if (cur != t->nil) {
    delete_node(t, cur->left);
    delete_node(t, cur->right);
    free(cur);
  }
}

void delete_rbtree(rbtree *t) {
  delete_node(t, t->root);
  free(t->nil);
  free(t);
}

void rotate_left(rbtree *t, node_t *cur) {
  if(cur == NULL)
    return;

  // cur : 회전의 중심축, r : 새로운 부모 노드
  node_t *r = cur->right;
  // 새부모노드의 왼쪽 자식을 중심축의 오른쪽 자식으로 삽입
  cur->right = r->left;

  // 삽입된 오른쪽 자식이 NIL노드가 아닐 경우, 부모를 갱신
  if (r->left != t->nil)
    r->left->parent = cur;

  // 중심축의 현재부모는 회전 후에는 할아버지(grandp)가 됨 ( = cur - r - grandp )
  r->parent = cur->parent;

  /* 
  if, 만약 중심축이 루트 노드 였다면, 새부모가 새로운 루트가 됨
  else if, 중심축이 루트노드가 아니었는데, 할아버지의 왼쪽 자식이 아직 중심축이면, 새부모로 갱신
  else, 위의 경우에 모두 해당되지 않으면 할아버지의 오른쪽 노드를 새부모로 갱신
  */
  if (cur->parent == t->nil)
    t->root = r;
  else if (cur == cur->parent->left)
    cur->parent->left = r;
  else
    cur->parent->right = r;

  // 부모자식 관계 갱신
  r->left = cur;
  cur->parent = r;
}

void rotate_right(rbtree *t, node_t *cur) {
  if (cur == NULL)
    return;

  // rotate_left 와 동일
  node_t *l = cur->left;
  cur->left = l->right;

  if (l->right != t->nil) {
    l->right->parent = cur;
  }

  l->parent = cur->parent;

  if (l->parent == t->nil)
    t->root = l;
  else if (cur == cur->parent->right)
    cur->parent->right = l;
  else
    cur->parent->left = l;

  l->right = cur;
  cur->parent = l;
}

void insert_fix(rbtree *t, node_t *cur) {
  if (cur == NULL)
    return;

  node_t *uncle;

  while (cur->parent != NULL && cur->parent->color == RBTREE_RED) {
    // 오른쪽 서브트리 수정
    if (cur->parent->parent != NULL && cur->parent == cur->parent->parent->right) {
      uncle = cur->parent->parent->left;
      // case 1r : 나의 부모의 형제(삼촌)가 빨간색일 때, 색만 바꿔줌
      if (uncle != NULL && uncle->color == RBTREE_RED) {
        uncle->color = RBTREE_BLACK;
        cur->parent->color = RBTREE_BLACK;
        cur->parent->parent->color = RBTREE_RED;
        cur = cur->parent->parent;
      }
      // case : 삼촌이 검은색
      else {
        // case 3r : 내 위치가 부모의 왼쪽 서브트리일 때, 총 두 번 회전 (Double Rotate)
        if (cur == cur->parent->left) {
          cur = cur->parent;
          rotate_right(t, cur);
        }
        // case 2r : 내 위치가 부모의 오른쪽 서브트리일 떼, 한 번만 회전 (Single Rotate)
        cur->parent->color = RBTREE_BLACK;
        if (cur->parent->parent != NULL) {
          cur->parent->parent->color = RBTREE_RED;
          rotate_left(t, cur->parent->parent);
        }
      }
    }
    // 왼쪽 서브트리 수정
    else {
      uncle = cur->parent->parent->right;

      // case 1l : 삼촌이 빨간색일 때, 색만 바꿔줌
      if (uncle != NULL && uncle->color == RBTREE_RED) {
        uncle->color = RBTREE_BLACK;
        cur->parent->color = RBTREE_BLACK;
        cur->parent->parent->color = RBTREE_RED;
        cur = cur->parent->parent;
      }
      // case : 삼촌이 검은색
      else {
        // case 3l : 내 위치가 부모의 오른쪽 서브트리일 때, 총 두 번 회전 필요 [한 줄 정렬 후 이동] (Double Rotate)
        if (cur = cur->parent->right) {
          cur = cur->parent;
          rotate_left(t, cur);
        }
        // case 2l : 내 위치가 부모의 왼쪽 서브트리일 때, (이미 정렬되었기 떄문에) 한 번만 이동 (Single Rotate)
        cur->parent->color = RBTREE_BLACK;
        if (cur->parent->parent != NULL) {
          cur->parent->parent->color = RBTREE_RED;
          rotate_right(t, cur->parent->parent);
        }
      }

      if (cur == t->root)
        break;
    }

    if (t->root != NULL)
      t->root->color = RBTREE_BLACK;
  }
}

node_t *rbtree_insert(rbtree *t, const key_t key) {
  node_t *node = (node_t *)calloc(1, sizeof(node_t));
  node->key = key;
  node->color= RBTREE_RED;
  node->parent = t->nil;
  node->left = t->nil;
  node->right = t->nil;

  node_t *parent_position = t->nil;
  node_t *insert_position = t->root;

  // 노드가 들어갈 수 있는 위치 검색
  while(insert_position != t->nil) {
    parent_position = insert_position;
    // key와 루트값을 비교, key가 작으면 왼쪽 서브트리, key가 크면 오른쪽 서브트리
    if(node->key < insert_position->key)
      insert_position = insert_position->left;
    else
      insert_position = insert_position->right;
  }
  
  /* 
  리프노드에 도달하여 while문 탈출하고, 현재 node가 key의 parent가 됨
  if, while문을 들어가지 못했다면 key가 root가 됨
  else if, key가 parent보다 크면 오른쪽 자식
  else, key가 parent보다 작으면 왼쪽 자식
  */ 
  node->parent = parent_position;
  if (parent_position == t->nil)
    t->root = node;
  else if (node->key < parent_position->key) 
    parent_position->left = node;
  else
    parent_position->right = node;

  // tree balance 확인
  if (node->parent == t->nil) {
    node->color = RBTREE_BLACK;
    return node;
  }
  if (node->parent->parent == NULL)
    return node;

  insert_fix(t, node);
  return node;
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  node_t *cur = t->root;

  while(cur != t->nil) {
    if (cur->key > key)
      cur = cur->left;
    else if (cur->key < key)
      cur = cur->right;
    else 
      return cur;
  }
  return NULL;
}

node_t *rbtree_min(const rbtree *t) {
  node_t *min = t->root;

  if (min == t->nil)
    return NULL;

  while (min->left != t->nil) {
    min = min->left;
  }

  return min;
}

node_t *rbtree_max(const rbtree *t) {
  node_t *max = t->root;

  if (max == t->nil)
    return NULL;

  while (max->right != t->nil) {
    max = max->right;
  }

  return max;
}

void transplant(rbtree *t, node_t *node_del, node_t *node_sub) {
  if (node_del == NULL || node_sub == NULL)
    return;

  if (node_del->parent == t->nil)
    t->root = node_sub;
  else if (node_del == node_del->parent->left)
    node_del->parent->left = node_sub;
  else
    node_del->parent = node_del->parent;

  node_sub->parent = node_del->parent;
}

node_t *subtree_find_min(rbtree *t, node_t *cur) {
  if (cur == NULL)
    return NULL;

  while (cur->left != t->nil)
    cur = cur->left;

  return cur;
}

void erase_fix(rbtree *t, node_t *cur) {
  if (cur == NULL)
    return;

  node_t *sibling;

  while (cur != t->root && cur->color == RBTREE_BLACK) {
    // case 1l : 내 위치가 left이고, 형제가 right이면서 rbtree black height 위반
    if (cur->parent && cur == cur->parent->left) {
      sibling = cur ->parent->right;
      // case 1.1l : 형제가 red
      if (sibling && sibling->color == RBTREE_RED) {
        sibling->color = RBTREE_BLACK;
        cur->parent->color = RBTREE_RED;
        rotate_left(t, cur->parent);
        sibling = cur->parent->right;
      }
      // case 1.2l : 형제와 자식이 모두 black
      if (sibling && sibling->left && sibling->right && sibling->left->color == RBTREE_BLACK && sibling->right->color == RBTREE_BLACK) {
        sibling->color = RBTREE_RED;
        cur = cur->parent;
      }
      else {
        // case 1.3l : 형제는 black, 형제의 오른쪽 자식만 black
        if (sibling && sibling->right && sibling->right->color == RBTREE_BLACK) {
          sibling->left->color = RBTREE_BLACK;
          sibling->color = RBTREE_RED;
          rotate_right(t, sibling);
          sibling = cur->parent->right;
        }

        // case 1.4l : 형제는 black, 형제의 오른쪽 자식이 red
        sibling->color = cur->parent->color;
        cur->parent->color = RBTREE_BLACK;
        sibling->right->color = RBTREE_BLACK;
        rotate_left(t, cur->parent);
        cur = t->root;
      }
    }
    // case 1r : 내 위치가 right이고, 형제가 left이면서 rbtree black height 위반
    else {
      sibling = cur->parent->left;
      // case 1.1r : 형제가 red
      if (sibling->color == RBTREE_RED) {
        sibling->color = RBTREE_BLACK;
        cur->parent->color = RBTREE_RED;
        rotate_right(t,  cur->parent);
        sibling = cur->parent->left;
      }
      // case 1.2r : 형제와 자식이 모두 black
      if (sibling && sibling->right && sibling->left && sibling->right->color == RBTREE_BLACK && sibling->left->color == RBTREE_BLACK) {
          sibling->color = RBTREE_RED;
          cur = cur->parent;
      }
      else {
        // case 1.3r : 형제는 black, 형제의 오른쪽 자식만 black
        if (sibling && sibling->left && sibling->left->color == RBTREE_BLACK) {
          sibling->right->color = RBTREE_BLACK;
          sibling->color = RBTREE_RED;
          rotate_left(t, sibling);
          sibling = cur->parent->left;
        }

        // case 1.4r : 형제는 black, 형제의 오른쪽 자식이 red
        sibling->color = cur->parent->color;
        cur->parent->color = RBTREE_BLACK;
        sibling->left->color = RBTREE_BLACK;
        rotate(t, cur->parent);
        cur = t->root;
      }
    }
  }

  cur->color = RBTREE_BLACK;
}

int rbtree_erase(rbtree *t, node_t *cur) {
  if (cur == NULL)
    return -1;

  node_t *node_rep;
  node_t *node_ref = cur;
  int o_color = node_ref->color;

  if (cur->left == t->nil) {
    node_rep = cur->right;
    if (node_rep)
      transplant(t, cur, cur->right);
  }
  else if (cur->right == t->nil) {
    node_rep = cur->left;
    if (node_rep)
      transplant(t, cur, cur->left);
  }
  else {
    node_ref = subtree_find_min(t, cur->right);

    if (node_ref) {
      o_color = node_ref->color;
      node_rep = node_ref->right;

      if(node_ref) {
        if (node_ref->parent == cur)
          node_rep->parent = node_ref;
        else {
        transplant(t, node_ref, node_ref->right);
        node_ref->right = cur->right;
        if (node_ref->right)
          node_ref->right->parent = node_ref;
        }
      }
      transplant(t, cur, node_ref);
      node_ref->left = cur->left;
      if (node_ref->left)
        node_ref->left->parent = node_ref;
      
      node_ref->color = cur->color;
    }
  }

  if (o_color == RBTREE_BLACK)
    erase_fix(t, node_rep);
  
  free(cur);

  return 0;
}

void inorder_travel(const rbtree *t, key_t *arr, const size_t n, node_t *node, int *count) {
  if (node == NULL)
    return;

  if (node == t->nil)
    return;

  inorder_travel(t, arr, n, node->left, count);
  if (*count < n) {
    arr[*count] = node->key;
    *count = *count + 1;
  }
  inorder_travel(t, arr, n, node->right, count);
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  int count = 0;
  node_t *start = t->root;
  inorder_travel(t, arr, n, start, &count);
  return 0;
}
