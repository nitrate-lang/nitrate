
import "C" {
    fn printf(format: *i8, ...): i32;
    fn malloc(size: u64): *void;
    fn free(ptr: *void);
}

group Node {
    data: i32,
    left: *Node,
    right: *Node,
}

fn node_init(ptr: %**Node, data: i32) {
    *ptr = malloc(20);

    let n: %*Node = *ptr;

    n.data = data;
    n.left = null;
    n.right = null;
    ret;
}

fn list_create(): *Node {
    let n: *Node;
    node_init(&n, 0);

    ret n;
}

fn list_append(head: %*Node, data: i32) {
    let n: %*Node;
    node_init(&n, data);

    let current: %*Node = head;
    while current.right != null {
        current = current.right;
    }

    current.right = n;
    n.left = current;
    ret;
}

fn list_insert(head: %*Node, data: i32, index: i32) {
    let n: %*Node;
    node_init(&n, data);

    let current: %*Node = head;
    let i: %i32 = 0;
    while current.right != null && i < index {
        current = current.right;
        i += 1;
    }

    n.right = current.right;
    n.left = current;
    current.right = n;
    ret;
}

fn list_reverse(head: %**Node) {
    let current: %*Node = *head;
    let prev: %*Node = null;
    let next: %*Node = null;

    while current != null {
        next = current.right;
        current.right = prev;
        current.left = next;
        prev = current;
        current = next;
    }

    *head = prev;
}

fn list_print(head: %*Node) {
    let current: %*Node = head;
    while current != null {
        printf("%d\n", current.data);
        current = current.right;
    }
    ret;
}

fn main(): i32 {
    let head: *Node = list_create();
    list_append(head, 1);
    list_append(head, 2);
    list_append(head, 3);

    list_insert(head, 4, 1);

    list_print(head);

    printf("Reversing list\n");

    list_reverse(&head);

    list_print(head);
}