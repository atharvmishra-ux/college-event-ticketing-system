#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EVENTS 10

struct Attendee {
    int id;
    char name[30];
    int type;
    int seat;
    int status;
};

struct Node {
    int index;
    struct Node *next;
};

struct Event {
    int id;
    char name[30];
    char date[15];
    char venue[30];
    int capacity;
    int *seats;
    struct Attendee *list;
    int count;
    struct Node *front;
    struct Node *rear;
    int waitCount;
};

struct Event events[MAX_EVENTS];
int eventCount = 0;
int nextId = 1001;

char typeName[3][10] = {"Student", "Regular", "VIP"};
int price[3] = {100, 250, 500};

int getNumber(int min, int max)
{
    int n;
    while (1) {
        if (scanf("%d", &n) != 1) {
            while (getchar() != '\n');
            printf("Enter a number: ");
        } else if (n < min || n > max) {
            printf("Enter a number between %d and %d: ", min, max);
        } else {
            return n;
        }
    }
}

int isValidDate(char *s)
{
    int i, day, month, year;
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    /* format check: DD-MM-YYYY (exactly 10 characters) */
    if (strlen(s) != 10)
        return 0;
    for (i = 0; i < 10; i++) {
        if (i == 2 || i == 5) {
            if (s[i] != '-')
                return 0;
        } else if (s[i] < '0' || s[i] > '9') {
            return 0;
        }
    }

    /* convert to numbers */
    day   = (s[0] - '0') * 10 + (s[1] - '0');
    month = (s[3] - '0') * 10 + (s[4] - '0');
    year  = (s[6] - '0') * 1000 + (s[7] - '0') * 100 +
            (s[8] - '0') * 10 + (s[9] - '0');

    if (year < 2000 || year > 2100)
        return 0;
    if (month < 1 || month > 12)
        return 0;

    /* leap year */
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
        daysInMonth[1] = 29;

    if (day < 1 || day > daysInMonth[month - 1])
        return 0;

    return 1;
}

void addEvent()
{
    if (eventCount == MAX_EVENTS) {
        printf("Cannot add more events.\n");
        return;
    }

    struct Event *e = &events[eventCount];
    int i;

    e->id = eventCount + 1;
    printf("Event name (one word, use _ for space): ");
    scanf("%29s", e->name);

    while (1) {
        printf("Date (DD-MM-YYYY): ");
        scanf("%14s", e->date);
        if (isValidDate(e->date))
            break;
        printf("Invalid date! Use DD-MM-YYYY (e.g. 05-03-2026).\n");
    }

    printf("Venue (one word): ");
    scanf("%29s", e->venue);
    printf("Capacity: ");
    e->capacity = getNumber(1, 1000);

    e->seats = (int *)malloc(e->capacity * sizeof(int));
    for (i = 0; i < e->capacity; i++)
        e->seats[i] = 0;

    e->list = NULL;
    e->count = 0;
    e->front = NULL;
    e->rear = NULL;
    e->waitCount = 0;

    eventCount++;
    printf("Event added with ID %d\n", e->id);
}

int countBooked(struct Event *e)
{
    int i, total = 0;
    for (i = 0; i < e->capacity; i++)
        if (e->seats[i] != 0)
            total++;
    return total;
}

void showEvents()
{
    int i;
    if (eventCount == 0) {
        printf("No events yet.\n");
        return;
    }
    printf("\nID  Name          Date        Venue       Booked/Capacity  Waitlist\n");
    printf("------------------------------------------------------------------\n");
    for (i = 0; i < eventCount; i++) {
        struct Event *e = &events[i];
        printf("%-3d %-13s %-11s %-11s %d/%-13d %d\n",
               e->id, e->name, e->date, e->venue,
               countBooked(e), e->capacity, e->waitCount);
    }
}

struct Event *chooseEvent()
{
    int id;
    if (eventCount == 0) {
        printf("No events yet. Add an event first.\n");
        return NULL;
    }
    showEvents();
    printf("Enter event ID: ");
    id = getNumber(1, MAX_EVENTS);
    if (id > eventCount) {
        printf("Event not found.\n");
        return NULL;
    }
    return &events[id - 1];
}

int findFreeSeat(struct Event *e)
{
    int i;
    for (i = 0; i < e->capacity; i++)
        if (e->seats[i] == 0)
            return i + 1;
    return 0;
}

void addToWaitlist(struct Event *e, int index)
{
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    newNode->index = index;
    newNode->next = NULL;

    if (e->rear == NULL) {
        e->front = newNode;
        e->rear = newNode;
    } else {
        e->rear->next = newNode;
        e->rear = newNode;
    }
    e->waitCount++;
}

int removeFront(struct Event *e)
{
    struct Node *temp;
    int index;

    if (e->front == NULL)
        return -1;

    temp = e->front;
    index = temp->index;
    e->front = temp->next;
    if (e->front == NULL)
        e->rear = NULL;
    free(temp);
    e->waitCount--;
    return index;
}

void removeFromWaitlist(struct Event *e, int index)
{
    struct Node *cur = e->front;
    struct Node *prev = NULL;

    while (cur != NULL) {
        if (cur->index == index) {
            if (prev == NULL)
                e->front = cur->next;
            else
                prev->next = cur->next;
            if (cur == e->rear)
                e->rear = prev;
            free(cur);
            e->waitCount--;
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

void bookTicket(struct Event *e)
{
    struct Attendee *a;
    int seat, i;

    e->list = (struct Attendee *)realloc(e->list, (e->count + 1) * sizeof(struct Attendee));
    a = &e->list[e->count];

    printf("Attendee name (one word): ");
    scanf("%29s", a->name);
    printf("Ticket types:\n");
    for (i = 0; i < 3; i++)
        printf("  %d. %s - Rs.%d\n", i + 1, typeName[i], price[i]);
    printf("Choose type: ");
    a->type = getNumber(1, 3) - 1;
    a->id = nextId;
    nextId++;

    seat = findFreeSeat(e);
    if (seat > 0) {
        a->seat = seat;
        a->status = 1;
        e->seats[seat - 1] = a->id;
        printf("Booking confirmed! ID: %d, Seat: %d, Amount: Rs.%d\n",
               a->id, seat, price[a->type]);
    } else {
        a->seat = 0;
        a->status = 2;
        addToWaitlist(e, e->count);
        printf("Event full. Added to waitlist at position %d. ID: %d\n",
               e->waitCount, a->id);
    }
    e->count++;
}

void cancelTicket(struct Event *e)
{
    int id, i, index = -1, seat, next;

    printf("Enter booking ID to cancel: ");
    id = getNumber(1, 1000000);

    for (i = 0; i < e->count; i++) {
        if (e->list[i].id == id) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("Booking not found.\n");
        return;
    }
    if (e->list[index].status == 0) {
        printf("Booking already cancelled.\n");
        return;
    }

    if (e->list[index].status == 2) {
        removeFromWaitlist(e, index);
        e->list[index].status = 0;
        printf("Waitlist booking cancelled.\n");
        return;
    }

    seat = e->list[index].seat;
    e->seats[seat - 1] = 0;
    e->list[index].status = 0;
    e->list[index].seat = 0;
    printf("Booking cancelled. Refund: Rs.%d\n", price[e->list[index].type]);

    next = removeFront(e);
    if (next != -1) {
        e->list[next].status = 1;
        e->list[next].seat = seat;
        e->seats[seat - 1] = e->list[next].id;
        printf("%s from the waitlist got seat %d.\n", e->list[next].name, seat);
    }
}

void showAttendees(struct Event *e)
{
    int i, found = 0;
    printf("\nConfirmed attendees of %s\n", e->name);
    printf("ID     Name           Type      Seat\n");
    printf("-------------------------------------\n");
    for (i = 0; i < e->count; i++) {
        if (e->list[i].status == 1) {
            printf("%-6d %-14s %-9s %d\n", e->list[i].id, e->list[i].name,
                   typeName[e->list[i].type], e->list[i].seat);
            found = 1;
        }
    }
    if (found == 0)
        printf("No confirmed attendees.\n");
}

void showWaitlist(struct Event *e)
{
    struct Node *temp = e->front;
    int position = 1;

    printf("\nWaitlist of %s\n", e->name);
    printf("Pos  ID     Name           Type\n");
    printf("--------------------------------\n");
    while (temp != NULL) {
        struct Attendee *a = &e->list[temp->index];
        printf("%-4d %-6d %-14s %s\n", position, a->id, a->name, typeName[a->type]);
        position++;
        temp = temp->next;
    }
    if (position == 1)
        printf("Waitlist is empty.\n");
}

void showReport(struct Event *e)
{
    int i, booked, revenue = 0, sold[3] = {0, 0, 0};

    for (i = 0; i < e->count; i++) {
        if (e->list[i].status == 1) {
            sold[e->list[i].type]++;
            revenue = revenue + price[e->list[i].type];
        }
    }
    booked = countBooked(e);

    printf("\n------ REPORT: %s ------\n", e->name);
    printf("Date     : %s\n", e->date);
    printf("Venue    : %s\n", e->venue);
    printf("Capacity : %d\n", e->capacity);
    printf("Booked   : %d\n", booked);
    printf("Available: %d\n", e->capacity - booked);
    printf("Waitlist : %d\n", e->waitCount);
    for (i = 0; i < 3; i++)
        printf("%-8s tickets: %d  (Rs.%d)\n", typeName[i], sold[i], sold[i] * price[i]);
    printf("Total revenue: Rs.%d\n", revenue);
}

char askAgain()
{
    char ans;
    while (1) {
        printf("\nDo you need to choose any other choice? (y/n): ");
        scanf(" %c", &ans);
        if (ans == 'y' || ans == 'Y')
            return 'y';
        if (ans == 'n' || ans == 'N')
            return 'n';
        printf("Please enter y or n.\n");
    }
}

void reportMenu(struct Event *e)
{
    int choice;
    char again = 'y';

    while (again == 'y') {
        printf("\n1. Event report and revenue\n");
        printf("2. Attendee list\n");
        printf("3. Waitlist\n");
        printf("0. Back\n");
        printf("Enter choice: ");
        choice = getNumber(0, 3);
        if (choice == 0)
            return;
        if (choice == 1)
            showReport(e);
        else if (choice == 2)
            showAttendees(e);
        else if (choice == 3)
            showWaitlist(e);
        again = askAgain();
    }
}

void freeMemory()
{
    int i;
    for (i = 0; i < eventCount; i++) {
        while (removeFront(&events[i]) != -1);
        free(events[i].seats);
        free(events[i].list);
    }
}

int main()
{
    int choice;
    char again = 'y';
    struct Event *e;

    printf("===== COLLEGE EVENT TICKETING SYSTEM =====\n");

    while (again == 'y') {
        printf("\n1. Add event\n");
        printf("2. Show events\n");
        printf("3. Book ticket\n");
        printf("4. Cancel ticket\n");
        printf("5. Reports\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        choice = getNumber(0, 5);

        if (choice == 0)
            break;

        if (choice == 1) {
            addEvent();
        } else if (choice == 2) {
            showEvents();
        } else if (choice == 3) {
            e = chooseEvent();
            if (e != NULL)
                bookTicket(e);
        } else if (choice == 4) {
            e = chooseEvent();
            if (e != NULL)
                cancelTicket(e);
        } else if (choice == 5) {
            e = chooseEvent();
            if (e != NULL)
                reportMenu(e);
        }
        again = askAgain();
    }

    freeMemory();
    printf("Thank you!\n");
    return 0;
}