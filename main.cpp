#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <windows.h>
using namespace std;

#define RED "\033[31m"
#define GREEN "\033[32m"
#define ORANGE "\033[38;5;214m"
#define RESET "\033[0m"

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

void print_euro(const wchar_t *str)
{
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), str, wcslen(str), NULL, NULL);
}

bool isValidDate(const string &dateStr)
{
    tm time = {};
    istringstream ss(dateStr);
    ss >> get_time(&time, "%d/%m/%Y");
    if (ss.fail())
    {
        cout << RED << "Invalid date format...\n"
             << RESET;
        return false;
    }
    return true;

    // auto parsedTime = chrono::system_clock::from_time_t(mktime(&time));
    // auto now = chrono::system_clock::now();
    // if (parsedTime > now)
    //     return true;
    // cout << "Enter an upcoming date...\n";
    // return false;
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

string getCurrentDate()
{
    auto now = std::time(nullptr);
    std::tm *localTime = std::localtime(&now);

    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%d/%m/%Y", localTime);
    return string(buffer);
}

userList *input_data(userList *mainlist)
{
    ifstream txtfile("database.txt");
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
                if (!isdigit(temp[i]) && temp[i] != '.')
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
    mainlist->tail = currentuser;
    return mainlist;
}

double convert(double amount, string sendercur, string targetcur)
{
    if (sendercur == "$" && targetcur == "€")
        return amount * 0.92;
    else if (sendercur == "$" && targetcur == "L.L")
        return amount * 90000;
    else if (sendercur == "L.L" && targetcur == "€")
        return amount / 90000 * 0.92;
    else if (sendercur == "L.L" && targetcur == "$")
        return amount / 90000;
    else if (sendercur == "€" && targetcur == "$")
        return amount / 0.92;
    else if (sendercur == "€" && targetcur == "L.L")
        return amount * 90000 / 0.92;
    return amount;
}

void transfer(userList *mainlist, double amount, account *sender, account *target)
{
    user *usercur = mainlist->head;
    account *acccur;
    transaction *txn;
    double sum = 0, sub;
    bool found = false;
    string curdate = getCurrentDate();

    while (usercur != NULL)
    {
        acccur = usercur->acct;
        while (acccur != NULL)
        {
            if (acccur->IBAN == sender->IBAN)
            {
                txn = acccur->txn;
                while (txn != NULL)
                {
                    if (txn->amount < 0 && txn->date.substr(3, 7) == curdate.substr(3, 7))
                    {
                        sum += -1 * txn->amount;
                    }
                    txn = txn->next;
                }
                if (acccur->balance < amount)
                {
                    cout << RED << "Insufficient balance in the sender account..." << RESET << endl;
                    return;
                }
                else if (acccur->limitWithdrawPerMonth < sum + amount)
                {
                    cout << RED << "Withdraw limit reached in the sender account..." << RESET << endl;
                    return;
                }
                else
                {
                    found = true;
                    break;
                }
            }
            acccur = acccur->next;
        }
        if (found)
            break;
        usercur = usercur->next;
    }
    if (!found)
    {
        cout << RED << "Sender account not found..." << RESET << endl;
        return;
    }
    found = false;
    sum = 0;
    usercur = mainlist->head;
    account *targetacc;
    double amount2;

    while (usercur != NULL)
    {
        targetacc = usercur->acct;
        while (targetacc != NULL)
        {
            if (targetacc->IBAN == target->IBAN)
            {
                amount2 = convert(amount, acccur->currency, targetacc->currency);
                while (txn != NULL)
                {
                    if (txn->amount > 0 && txn->date == curdate)
                    {
                        sum += txn->amount;
                    }
                    txn = txn->next;
                }
                if (targetacc->limitDepositPerDay >= sum + amount2)
                {
                    found = true;
                    break;
                }
                else
                {
                    cout << RED << "Deposit limit exceeded in target account..." << RESET << endl;
                    return;
                }
            }
            targetacc = targetacc->next;
        }
        if (found)
            break;
        usercur = usercur->next;
    }
    if (!found)
    {
        cout << RED << "Target account not found..." << RESET << endl;
        return;
    }
    else
    {
        transaction *send = new transaction;
        transaction *receive = new transaction;

        acccur->balance -= amount;
        send->amount = -amount;
        send->date = curdate;
        send->next = acccur->txn;
        acccur->txn = send;

        targetacc->balance += amount2;
        receive->amount = amount2;
        receive->date = curdate;
        receive->next = targetacc->txn;
        targetacc->txn = receive;
    }

    cout << GREEN << "Transaction completed successfully..." << RESET << endl;
}

userList *sort(userList *mainlist)
{
    user *usercur = mainlist->head;

    while (usercur != NULL)
    {
        account *acccur = usercur->acct;
        while (acccur != NULL)
        {
            transaction *head = acccur->txn;
            if (!head)
            {
                acccur = acccur->next;
                continue;
            }

            transaction *sorted = NULL;

            while (head != sorted)
            {
                transaction *cur = head;
                transaction *prev = NULL;

                while (cur->next != sorted)
                {
                    if (!compareDate(cur->date, cur->next->date))
                    {
                        transaction *next = cur->next;
                        cur->next = next->next;
                        next->next = cur;

                        if (prev)
                            prev->next = next;
                        else
                            head = next;

                        prev = next;
                    }
                    else
                    {
                        prev = cur;
                        cur = cur->next;
                    }
                }
                sorted = cur;
            }
            acccur->txn = head;
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
                cout << GREEN << "Account added successfully..." << RESET << endl;
                usercur->acct = newacc;
                return mainlist;
            }

            while (acccur->next != NULL)
            {
                if (acccur->IBAN == newacc->IBAN)
                {
                    cout << RED << "Account already exists..." << RESET << endl;
                    return mainlist;
                }
                acccur = acccur->next;
            }

            if (acccur->IBAN == newacc->IBAN)
            {
                cout << RED << "Account already exists..." << RESET << endl;
                return mainlist;
            }

            acccur->next = newacc;
            cout << GREEN << "Account added successfully..." << RESET << endl;
            return mainlist;
        }

        usercur = usercur->next;
    }

    cout << RED << "User not found..." << RESET << endl;
    return mainlist;
}

userList *create_account(userList *mainlist)
{
    user *current = mainlist->head;
    user *temp;
    account *acccur;
    bool error;
    int i = 0, input;
    if (current == NULL)
    {
        cout << RED << "No users available, please create a user before proceeding..." << RESET << endl;
        return mainlist;
    }
    cout << setw(5) << "ID" << setw(30) << "Name" << endl;
    while (current != NULL)
    {
        cout << setw(5) << current->userID << setw(30) << current->fname + " " + current->lname << endl;
        i++;
        current = current->next;
    }
    error = false;
    do
    {
        if (error)
        {
            cout << RED << "Invalid ID..." << RESET << endl;
        }
        cout << "Enter the user ID that you want to create a new account to: \n";
        cout << ORANGE << "-> " << RESET;
        error = true;
        cin >> input;
    } while (input <= 0 || input > i);

    current = mainlist->head;
    while (current != NULL)
    {
        if (current->userID == input)
        {
            temp = current;
            break;
        }
        current = current->next;
    }

    account *newacc = new account;
    cin.ignore();
    error = false;
    do
    {
        if (error)
        {
            cout << RED << "Invalid input..." << RESET << endl;
        }
        cout << "Enter the IBAN of your account...\n";
        cout << ORANGE << "-> " << RESET;
        getline(cin, newacc->IBAN);
        for (int i = 0; i < newacc->IBAN.length(); i++)
        {
            newacc->IBAN[i] = toupper(newacc->IBAN[i]);
        }
        current = mainlist->head;
        error = true;
        while (current != NULL)
        {
            acccur = current->acct;
            while (acccur != NULL)
            {
                if (acccur->IBAN == newacc->IBAN)
                {
                    error = false;
                    cout << RED << "This IBAN already belongs to another account..." << RESET << endl;
                }
                acccur = acccur->next;
            }
            current = current->next;
        }

    } while (newacc->IBAN == "" || error == false);

    error = false;
    do
    {
        if (error)
        {
            cout << RED << "Invalid input..." << RESET << endl;
        }
        cout << "Enter the name of your account...\n";
        cout << ORANGE << "-> " << RESET;
        getline(cin, newacc->accountName);
        error = true;

    } while (newacc->accountName == "");

    error = false;
    do
    {
        if (error)
        {
            cout << RED << "Invalid input..." << RESET << endl;
        }
        cout << "Enter the currency of your account...\n";
        cout << ORANGE << "-> " << RESET;
        getline(cin, newacc->currency);
        if (newacc->currency == "?")
            newacc->currency = "€";
        error = true;

    } while (newacc->currency != "€" && newacc->currency != "$" && newacc->currency != "L.L");

    error = false;
    do
    {
        if (error)
        {
            cout << RED << "Invalid input..." << RESET << endl;
        }
        cout << "Enter the balance of your account...\n";
        cout << ORANGE << "-> " << RESET;
        cin >> newacc->balance;
        error = true;

    } while (newacc->balance < 0);

    error = false;
    do
    {
        if (error)
        {
            cout << RED << "Invalid input..." << RESET << endl;
        }
        cout << "Enter the limit deposit per day of your account...\n";
        cout << ORANGE << "-> " << RESET;
        cin >> newacc->limitDepositPerDay;
        error = true;

    } while (newacc->limitDepositPerDay <= 0);

    error = false;
    do
    {
        if (error)
        {
            cout << RED << "Invalid input..." << RESET << endl;
        }
        cout << "Enter the limit withdraw per month of your account...\n";
        cout << ORANGE << "-> " << RESET;
        cin >> newacc->limitWithdrawPerMonth;
        error = true;

    } while (newacc->limitWithdrawPerMonth <= 0);

    return add_account(mainlist, temp, newacc);
}

userList *deposit_withdraw(userList *mainlist, int choice)
{
    string IBAN;
    double amount;
    double total = 0;
    account *acccur;
    transaction *txncur;
    bool found = false;
    user *usercur = mainlist->head;
    string today = getCurrentDate();
    cout << "Enter the IBAN of the account:\n";
    cout << ORANGE << "-> " << RESET;
    getline(cin, IBAN);
    for (int i = 0; i < IBAN.length(); i++)
    {
        IBAN[i] = toupper(IBAN[i]);
    }

    do
    {
        if (choice == 1)
            cout << "Enter the amount to deposit:" << endl;
        else
            cout << "Enter the amount to withdraw:" << endl;
        cout << ORANGE << "-> " << RESET;
        cin >> amount;
    } while (amount < 0);
    total += amount;
    while (usercur != NULL)
    {
        acccur = usercur->acct;
        while (acccur != NULL)
        {
            if (acccur->IBAN == IBAN)
            {
                found = true;
                break;
            }
            acccur = acccur->next;
        }
        if (found)
        {
            break;
        }
        usercur = usercur->next;
    }
    if (!found)
    {
        cout << RED << "Account not found..." << RESET << endl;
        return mainlist;
    }

    if (choice == 1)
    {
        txncur = acccur->txn;
        while (txncur != NULL)
        {
            if (txncur->amount > 0 && txncur->date == today)
            {
                total += txncur->amount;
            }
            txncur = txncur->next;
        }
        if (total > acccur->limitDepositPerDay)
        {
            cout << RED << "Limit deposit per day reached..." << RESET << endl;
        }
        else
        {
            acccur->balance += amount;
            transaction *newtxn = new transaction;
            newtxn->date = today;
            newtxn->amount = amount;
            newtxn->next = NULL;
            if (acccur->txn == NULL)
            {
                acccur->txn = newtxn;
            }
            else
            {
                txncur = acccur->txn;
                while (txncur->next != NULL)
                {
                    txncur = txncur->next;
                }
                txncur->next = newtxn;
            }
            cout << GREEN << "Amount deposited successfully..." << RESET << endl;
        }
    }
    else
    {
        txncur = acccur->txn;

        while (txncur != NULL)
        {
            if (txncur->amount < 0 && txncur->date.substr(3, 7) == today.substr(3, 7))
            {
                total += -1 * txncur->amount;
            }
            txncur = txncur->next;
        }
        if (total > acccur->limitWithdrawPerMonth)
        {
            cout << RED << "Limit withdraw per month reached..." << RESET << endl;
        }
        else
        {
            acccur->balance -= amount;
            transaction *newtxn = new transaction;
            newtxn->date = today;
            newtxn->amount = -1 * amount;
            newtxn->next = NULL;
            if (acccur->txn == NULL)
            {
                acccur->txn = newtxn;
            }
            else
            {
                txncur = acccur->txn;
                while (txncur->next != NULL)
                {
                    txncur = txncur->next;
                }
                txncur->next = newtxn;
            }
            cout << GREEN << "Amount withdrawn successfully..." << RESET << endl;
        }
    }
    return mainlist;
}

userList *create_user(userList *mainlist)
{
    int i = 0;
    bool error;
    user *newuser = new user;
    if (mainlist->head != NULL)
    {
        i = mainlist->tail->userID + 1;
    }

    newuser->userID = i;

    error = false;
    do
    {
        if (error)
        {
            cout << RED << "Invalid input..." << RESET << endl;
        }
        cout << "Enter your first name...\n";
        cout << ORANGE << "-> " << RESET;
        getline(cin, newuser->fname);
        error = true;

    } while (newuser->fname == "");

    error = false;
    do
    {
        if (error)
        {
            cout << RED << "Invalid input..." << RESET << endl;
        }
        cout << "Enter your last name...\n";
        cout << ORANGE << "-> " << RESET;
        getline(cin, newuser->lname);
        error = true;

    } while (newuser->lname == "");
    newuser->next = NULL;
    newuser->acct = NULL;
    if (mainlist->head == NULL)
    {
        mainlist->head = newuser;
        mainlist->tail = newuser;
    }
    else
    {
        mainlist->tail->next = newuser;
        newuser->previous = mainlist->tail;
        mainlist->tail = newuser;
    }
    cout << GREEN << "User added successfully..." << RESET << endl;
    return mainlist;
}

int display_accounts(userList *mainlist)
{
    user *usercur = mainlist->head;
    account *acccur;
    transaction *txncur;
    bool accounts = true;
    int i = 1;
    if (usercur == NULL)
    {
        cout << RED << "No users available, please create a user before proceeding..." << RESET << endl;
        return 0;
    }
    cout << ORANGE << setw(5) << "NB" << setw(10) << "IBAN" << setw(15) << "Name" << setw(10) << "Currency" << setw(10) << "Balance" << setw(10) << "LD/D" << setw(10) << "LW/M" << RESET << endl;
    while (usercur != NULL)
    {
        acccur = usercur->acct;
        while (acccur != NULL)
        {
            accounts = false;
            cout << setw(5) << i++ << setw(10) << acccur->IBAN << setw(15) << acccur->accountName;
            if (acccur->currency != "$" && acccur->currency != "L.L")
            {
                cout << "         ";
                print_euro(L"€");
            }
            else
            {
                cout << setw(10) << acccur->currency;
            }
            if (static_cast<int>(acccur->balance) == acccur->balance)
                cout << fixed << setprecision(0) << setw(10) << acccur->balance;
            else
                cout << fixed << setprecision(1) << setw(10) << acccur->balance;
            if (static_cast<int>(acccur->limitDepositPerDay) == acccur->limitDepositPerDay)
                cout << fixed << setprecision(0) << setw(10) << acccur->limitDepositPerDay;
            else
                cout << fixed << setprecision(1) << setw(10) << acccur->limitDepositPerDay;
            if (static_cast<int>(acccur->limitWithdrawPerMonth) == acccur->limitWithdrawPerMonth)
                cout << fixed << setprecision(0) << setw(10) << acccur->limitWithdrawPerMonth << endl;
            else
                cout << fixed << setprecision(1) << setw(10) << acccur->limitWithdrawPerMonth << endl;
            acccur = acccur->next;
        }
        usercur = usercur->next;
    }
    if (accounts)
    {
        cout << RED << "No accounts found..." << RESET << endl;
        return 0;
    }
    return 1;
}

userList *delete_transactions(userList *mainlist, string date)
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
                previous = acccur->txn;
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
                    acccur->txn = transcur->next;
                    delete transcur;
                }
                acccur = acccur->next;
            }
        }

        usercur = usercur->next;
    }
    return mainlist;
}

void export_data(userList *mainlist)
{
    ofstream txtfile("database.txt");
    user *usercur = mainlist->head;
    account *acccur;
    transaction *txncur;
    while (usercur != NULL)
    {
        txtfile << "-" << usercur->userID << "," << usercur->fname << "," << usercur->lname << "\n";
        acccur = usercur->acct;
        while (acccur != NULL)
        {
            txtfile << "#" << acccur->IBAN << "," << acccur->accountName << ",";
            if (static_cast<int>(acccur->balance) == acccur->balance)
                txtfile << fixed << setprecision(0) << acccur->balance << acccur->currency << ",";
            else
                txtfile << fixed << setprecision(1) << acccur->balance << acccur->currency << ",";
            if (static_cast<int>(acccur->limitDepositPerDay) == acccur->limitDepositPerDay)
                txtfile << fixed << setprecision(0) << acccur->limitDepositPerDay << acccur->currency << ",";
            else
                txtfile << fixed << setprecision(1) << acccur->limitDepositPerDay << acccur->currency << ",";
            if (static_cast<int>(acccur->limitWithdrawPerMonth) == acccur->limitWithdrawPerMonth)
                txtfile << fixed << setprecision(0) << acccur->limitWithdrawPerMonth << acccur->currency << "\n";
            else
                txtfile << fixed << setprecision(1) << acccur->limitWithdrawPerMonth << acccur->currency << "\n";
            txncur = acccur->txn;
            while (txncur != NULL)
            {
                txtfile << "*" << txncur->date << ",";
                if (static_cast<int>(txncur->amount) == txncur->amount)
                    txtfile << fixed << setprecision(0) << txncur->amount << acccur->currency << "\n";
                else
                    txtfile << fixed << setprecision(1) << txncur->amount << acccur->currency << "\n";
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
    mainlist = input_data(mainlist);
    int input;
    char inputchar;
    bool error;
    string date;
    double amount;
    cout << "\nWelcome to Cedars Bank...\n";
    mainlist = sort(mainlist);
    while (true)
    {
        error = false;
        export_data(mainlist);
        do
        {
            if (error)
            {
                cout << RED << "Invalid input..." << RESET << endl;
            }
            cout << "How can we help you?\n";
            cout << ORANGE << setw(5) << 1 << RESET << " Create a new user...\n";
            cout << ORANGE << setw(5) << 2 << RESET << " Create a new account...\n";
            cout << ORANGE << setw(5) << 3 << RESET << " Deposit/Withdraw money from account...\n";
            cout << ORANGE << setw(5) << 4 << RESET << " Transfer money to another account...\n";
            cout << ORANGE << setw(5) << 5 << RESET << " Delete transactions before a specified date...\n";
            cout << ORANGE << setw(5) << 0 << RESET << " Exit application...\n";
            cout << ORANGE << "-> " << RESET;
            cin >> input;
            error = true;
        } while (input < 0 || input > 5);
        cin.ignore();
        if (input == 0)
        {
            break;
        }
        else if (input == 1)
        {
            mainlist = create_user(mainlist);
        }
        else if (input == 2)
        {
            mainlist = create_account(mainlist);
        }
        else if (input == 3)
        {
            error = false;
            do
            {
                if (error)
                {
                    cout << RED << "Invalid input..." << RESET << endl;
                }
                cout << ORANGE << setw(5) << 1 << RESET << " Deposit money to account...\n";
                cout << ORANGE << setw(5) << 2 << RESET << " Withdraw money from account...\n";
                cout << ORANGE << "-> " << RESET;
                cin >> input;
                error = true;
            } while (input < 1 || input > 2);
            cin.ignore();
            if (display_accounts(mainlist) == 1)
            {
                mainlist = deposit_withdraw(mainlist, input);
            }
        }
        else if (input == 4)
        {
            input = display_accounts(mainlist);
            if (input == 1)
            {
                account acc1, acc2;
                cout << "Enter the IBAN of the sender:\n";
                cout << ORANGE << "-> " << RESET;
                getline(cin, acc1.IBAN);
                for (int i = 0; i < acc1.IBAN.length(); i++)
                {
                    acc1.IBAN[i] = toupper(acc1.IBAN[i]);
                }
                cout << "Enter the IBAN of the receiver:\n";
                cout << ORANGE << "-> " << RESET;
                getline(cin, acc2.IBAN);
                for (int i = 0; i < acc2.IBAN.length(); i++)
                {
                    acc2.IBAN[i] = toupper(acc2.IBAN[i]);
                }
                do
                {
                    cout << "Enter the amount to send:" << endl;
                    cout << ORANGE << "-> " << RESET;
                    cin >> amount;
                } while (amount < 0);
                transfer(mainlist, amount, &acc1, &acc2);
            }
        }
        else if (input == 5)
        {
            do
            {
                cout << "Enter a specified date (dd/mm/yyyy)\n";
                cout << ORANGE << "-> " << RESET;
                getline(cin, date);
            } while (!isValidDate(date));

            error = false;
            do
            {
                if (error)
                {
                    cout << RED << "Invalid input..." << RESET << endl;
                }
                cout << "This action cannot be undone, do you want to proceed? [y/n]: " << endl;
                cout << ORANGE << "-> " << RESET;
                cin >> inputchar;
                error = true;
            } while (tolower(inputchar) != 'y' && tolower(inputchar) != 'n');

            if (tolower(inputchar) == 'y')
            {
                cout << GREEN << "Proceeding..." << endl;
                mainlist = delete_transactions(mainlist, date);
                cout << "All transactions before " << date << " were successfully deleted..." << RESET << endl;
            }
        }
    }
    cout << GREEN << "Thanks for using our services...\n";
    cout << "Exiting app..." << RESET << endl;
    mainlist = sort(mainlist);
    export_data(mainlist);
}