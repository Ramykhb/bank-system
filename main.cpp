#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
using namespace std;

struct transaction
{
    string date;
    double amount;
    transaction *next;
};

struct account
{
    string IBAN;
    string accountName;
    double balance;
    string currency;
    double limitDepositPerDay;
    double limitWithdrawPerMonth;
    transaction *txn;
    account *next;
};

struct user
{
    int userID;
    string fname;
    string lname;
    account *acct;
    user *next, *previous;
};

struct userList
{
    user *head, *tail;
};

bool isValidDate(const string &dateStr)
{
    tm time = {};
    istringstream ss(dateStr);
    ss >> get_time(&time, "%d/%m/%Y");
    if (ss.fail())
    {
        cout << "Invalid date format...\n";
        return false;
    }

    auto parsedTime = chrono::system_clock::from_time_t(mktime(&time));
    auto now = chrono::system_clock::now();
    if (parsedTime > now)
        return true;
    cout << "Enter an upcoming date...\n";
    return false;
}

bool compareDate(const string &dateStr, const string &dateStr1)
{
    tm time1 = {}, time2 = {};
    istringstream ss1(dateStr), ss2(dateStr1);
    ss1 >> get_time(&time1, "%d/%m/%Y");
    ss2 >> get_time(&time2, "%d/%m/%Y");

    auto tp1 = chrono::system_clock::from_time_t(mktime(&time1));
    auto tp2 = chrono::system_clock::from_time_t(mktime(&time2));
    if (tp1 <= tp2)
        return true;
    return false;
}

userList *input_data(userList *mainlist)
{
    ifstream txtfile("input.txt");
    string line, temp;
    bool firstuser = true;
    bool firstacc = true;
    bool firsttxn = true;
    user *currentuser;
    account *currentacc;
    transaction *currenttxn;
    int i;
    while (getline(txtfile, line))
    {
        istringstream iss(line);
        if (line[0] == '-')
        {
            firstacc = true;
            user *newuser = new user;

            if (!firstuser)
            {
                currentuser->next = newuser;
                newuser->previous = currentuser;
            }
            else
            {
                firstuser = false;
                mainlist->head = newuser;
                newuser->previous = NULL;
            }
            currentuser = newuser;

            getline(iss, temp, ',');
            currentuser->userID = stoi(line.substr(1, temp.length() - 1));
            getline(iss, currentuser->fname, ',');
            getline(iss, currentuser->lname, '\n');
            currentuser->next = NULL;
            currentuser->acct = NULL;
        }
        else if (line[0] == '#')
        {
            account *newacc = new account;
            firsttxn = true;

            if (!firstacc)
                currentacc->next = newacc;
            else
            {
                firstacc = false;
                currentuser->acct = newacc;
            }
            currentacc = newacc;
            getline(iss, temp, ',');
            currentacc->IBAN = temp.substr(1, temp.length() - 1);

            getline(iss, currentacc->accountName, ',');

            getline(iss, temp, ',');
            for (i = 0; i < temp.length(); i++)
            {
                if (!isdigit(temp[i]))
                {
                    break;
                }
            }
            currentacc->balance = stod(temp.substr(0, i));
            currentacc->currency = temp.substr(i, temp.length() - i);

            getline(iss, temp, ',');
            for (i = 0; i < temp.length(); i++)
            {
                if (!isdigit(temp[i]))
                {
                    break;
                }
            }
            currentacc->limitDepositPerDay = stod(temp.substr(0, i));

            getline(iss, temp, '\n');
            for (i = 0; i < temp.length(); i++)
            {
                if (!isdigit(temp[i]))
                {
                    break;
                }
            }
            currentacc->limitWithdrawPerMonth = stod(temp.substr(0, i));
            currentacc->next = NULL;
            currentacc->txn = NULL;
        }

        else if (line[0] == '*')
        {
            transaction *newtxn = new transaction;
            if (!firsttxn)
                currenttxn->next = newtxn;
            else
            {
                firsttxn = false;
                currentacc->txn = newtxn;
            }
            currenttxn = newtxn;

            getline(iss, temp, ',');
            currenttxn->date = temp.substr(1, temp.length() - 1);
            getline(iss, temp, '\n');
            for (i = 1; i < temp.length(); i++)
            {
                if (!isdigit(temp[i]))
                {
                    break;
                }
            }
            currenttxn->amount = stod(temp.substr(0, i));
            currenttxn->next = NULL;
        }
        else
        {
            cout << "Invalid input file..." << endl;
            exit(1);
        }
    }
    cout << currentuser->fname;
    mainlist->tail = currentuser;
    return mainlist;
}

void transfer(userList *mainlist, double amount, account *sender, account *target)
{
    // Ma 3refet sewe shi
    // hahahaha
}

userList *sort(userList *mainlist)
{
    user *usercur = mainlist->head;
    account *acccur;
    transaction *txn, *head;
    transaction *prev, *temp, *maxdate, *tprev;
    bool swap;

    while (usercur != NULL)
    {
        acccur = usercur->acct;
        while (acccur != NULL)
        {
            txn = acccur->txn;
            head = txn;
            maxdate = txn;
            while (txn != NULL)
            {
                prev = txn;
                temp = txn->next;
                swap = false;
                while (temp != NULL)
                {
                    if (compareDate(maxdate->date, temp->date))
                    {
                        tprev = prev;
                        maxdate = temp;
                        swap = true;
                    }
                    prev = prev->next;
                    temp = temp->next;
                }
                if (swap && compareDate(txn->date, temp->date))
                {
                    tprev->next = maxdate->next;
                    maxdate->next = head;
                }
                else if (swap && txn != head)
                {
                    tprev->next = txn->next;
                    maxdate = txn;
                    maxdate->next = head;
                }
                txn = txn->next;
            }
            acccur = acccur->next;
        }
        usercur = usercur->next;
    }

    return mainlist;
}

userList *add_account(userList *mainlist, user *client, account *temp)
{
    user *usercur = mainlist->head;
    account *acccur;
    account *newacc = new account;
    newacc = temp;
    newacc->next = NULL;
    while (usercur != NULL)
    {
        if (usercur == client)
        {
            acccur = usercur->acct;
            if (acccur == NULL)
            {
                cout << "Account added successfully..." << endl;
                usercur->acct = newacc;
                return mainlist;
            }

            while (acccur->next != NULL)
            {
                if (acccur->IBAN == newacc->IBAN)
                {
                    cout << "Account already exists..." << endl;
                    return mainlist;
                }
                acccur = acccur->next;
            }

            if (acccur->IBAN == newacc->IBAN)
            {
                cout << "Account already exists..." << endl;
                return mainlist;
            }

            acccur->next = newacc;
            cout << "Account added successfully..." << endl;
            return mainlist;
        }

        usercur = usercur->next;
    }

    cout << "User not found..." << endl;
    return mainlist;
}

void delete_transactions(userList *mainlist, string date)
{
    user *usercur = mainlist->head;
    account *acccur;
    transaction *transcur;
    transaction *previous;

    while (usercur != NULL)
    {
        acccur = usercur->acct;
        while (acccur != NULL)
        {
            if (acccur->txn != NULL)
            {
                transcur = acccur->txn->next;
                while (transcur != NULL)
                {
                    if (compareDate(transcur->date, date))
                    {
                        previous->next = transcur->next;
                        delete transcur;
                        transcur = previous;
                    }
                    previous = transcur;
                    transcur = transcur->next;
                }
                transcur = acccur->txn;
                if (compareDate(transcur->date, date))
                {
                    acccur->txn->next = transcur->next;
                    delete transcur;
                }
                acccur = acccur->next;
            }
        }

        usercur = usercur->next;
    }
}

void export_data(userList *mainlist)
{
    ofstream txtfile("output.txt");
    user *usercur = mainlist->head;
    account *acccur;
    transaction *txncur;
    cout << 1 << mainlist->head->fname;
    while (usercur != NULL)
    {
        txtfile << "-" << usercur->userID << "," << usercur->fname << "," << usercur->lname << "\n";
        acccur = usercur->acct;
        while (acccur != NULL)
        {
            txtfile << "#" << acccur->IBAN << "," << acccur->accountName << "," << acccur->balance << acccur->currency << "," << acccur->limitDepositPerDay << acccur->currency << "," << acccur->limitWithdrawPerMonth << acccur->currency << "\n";
            txncur = acccur->txn;
            while (txncur != NULL)
            {
                txtfile << "*" << txncur->date << "," << txncur->amount << acccur->currency << "\n";
                txncur = txncur->next;
            }
            acccur = acccur->next;
        }
        usercur = usercur->next;
    }
    txtfile.close();
}

int main()
{
    userList *mainlist = new userList;

    /* mainlist = input_data(mainlist);
    account *temp = new account;
    temp->accountName = "ramy";
    temp->balance = 500;
    temp->limitDepositPerDay = 500;
    temp->limitWithdrawPerMonth = 500;                      ->Testing l add account
    temp->currency = "usd";
    temp->txn = NULL;
    temp->IBAN = "R123";
    cout << mainlist->head->fname;
    mainlist = add_account(mainlist, mainlist->head->next, temp); */

    export_data(mainlist);
}