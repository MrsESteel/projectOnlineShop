#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
using namespace std;

class Product {
    int id_;
    string name_, description_;
    double price_;
    int stock_;
public:
    Product(int id, string name, string description, double price, int stock)
        : id_(id), name_(name), description_(description), price_(price), stock_(stock) {
    }
    int id() const { return id_; }
    string name() const { return name_; }
    string description() const { return description_; }
    double price() const { return price_; }
    int stock() const { return stock_; }
    void update(string name, string description, double price, int stock) {
        name_ = name; description_ = description; price_ = price; stock_ = stock;
    }
    bool reduceStock(int quantity) {
        if (quantity <= 0 || stock_ < quantity) return false;
        stock_ -= quantity;
        return true;
    }
};

class Catalog {
    unordered_map<int, Product> products_;
    mutable mutex mutex_;
public:
    bool addProduct(int id, const string& name, const string& description, double price, int stock) {
        lock_guard<mutex> lock(mutex_);
        if (id <= 0 || name.empty() || price < 0 || stock < 0 || products_.count(id)) return false;
        products_.emplace(id, Product(id, name, description, price, stock));
        return true;
    }
    bool removeProduct(int id) {
        lock_guard<mutex> lock(mutex_);
        return products_.erase(id) > 0;
    }
    bool updateProduct(int id, const string& name, const string& description, double price, int stock) {
        lock_guard<mutex> lock(mutex_);
        auto it = products_.find(id);
        if (it == products_.end() || name.empty() || price < 0 || stock < 0) return false;
        it->second.update(name, description, price, stock);
        return true;
    }
    void display() const {
        lock_guard<mutex> lock(mutex_);
        cout << "\n--- PRODUCT CATALOG ---\n";
        for (const auto& entry : products_) {
            const Product& p = entry.second;
            cout << p.id() << ". " << p.name() << " - $" << fixed << setprecision(2)
                << p.price() << " | Stock: " << p.stock() << " | " << p.description() << '\n';
        }
    }
    bool getProduct(int id, Product& result) const {
        lock_guard<mutex> lock(mutex_);
        auto it = products_.find(id);
        if (it == products_.end()) return false;
        result = it->second;
        return true;
    }
    // Validate all items before changing any inventory, under one lock.
    bool purchase(const unordered_map<int, int>& items, double discount, double& total) {
        lock_guard<mutex> lock(mutex_);
        if (items.empty()) return false;
        double subtotal = 0;
        for (const auto& item : items) {
            auto it = products_.find(item.first);
            if (it == products_.end() || item.second <= 0 || it->second.stock() < item.second) return false;
            subtotal += it->second.price() * item.second;
        }
        for (const auto& item : items) products_.at(item.first).reduceStock(item.second);
        total = subtotal * (1.0 - discount);
        return true;
    }
};

class ShoppingCart {
    unordered_map<int, int> items_;
public:
    bool add(int id, int quantity) {
        if (id <= 0 || quantity <= 0 || quantity > 10000) return false;
        auto existing = items_.find(id);
        if (existing != items_.end() && existing->second > 10000 - quantity) return false;
        items_[id] += quantity;
        return true;
    }
    bool remove(int id) { return items_.erase(id) > 0; }
    bool updateQuantity(int id, int quantity) {
        if (!items_.count(id) || quantity < 0 || quantity > 10000) return false;
        if (quantity == 0) items_.erase(id);
        else items_[id] = quantity;
        return true;
    }
    double calculateTotal(const Catalog& catalog) const {
        double total = 0;
        for (const auto& item : items_) {
            Product p(0, "", "", 0, 0);
            if (catalog.getProduct(item.first, p)) total += p.price() * item.second;
        }
        return total;
    }
    const unordered_map<int, int>& items() const { return items_; }
    void clear() { items_.clear(); }
    void display(const Catalog& catalog) const {
        cout << "\n--- YOUR CART ---\n";
        if (items_.empty()) cout << "Cart is empty.\n";
        for (const auto& item : items_) {
            Product p(0, "", "", 0, 0);
            if (catalog.getProduct(item.first, p))
                cout << p.name() << " x" << item.second << " = $" << fixed << setprecision(2)
                << p.price() * item.second << '\n';
            else cout << "Product " << item.first << " is no longer available.\n";
        }
        cout << "Subtotal: $" << fixed << setprecision(2) << calculateTotal(catalog) << '\n';
    }
};

class User {
    string name_, email_, address_;
    ShoppingCart cart_;
public:
    User(string name, string email, string address)
        : name_(name), email_(email), address_(address) {
    }
    virtual ~User() = default;
    virtual double discount() const { return 0.0; }
    virtual string membership() const { return "Regular"; }
    string name() const { return name_; }
    string email() const { return email_; }
    string address() const { return address_; }
    ShoppingCart& cart() { return cart_; }
};
class PremiumUser : public User {
public:
    using User::User;
    double discount() const override { return 0.10; }
    string membership() const override { return "Premium"; }
};
class VIPUser : public User {
public:
    using User::User;
    double discount() const override { return 0.20; }
    string membership() const override { return "VIP"; }
};

int readInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) return value;
        if (cin.eof()) { cout << "Input closed.\n"; exit(0); }
        cout << "Please enter a whole number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

void checkout(User& user, Catalog& catalog) {
    if (user.cart().items().empty()) { cout << "Your cart is empty.\n"; return; }
    user.cart().display(catalog);
    cout << "Membership discount: " << user.discount() * 100 << "%\n";
    cout << "Estimated total: $" << fixed << setprecision(2)
        << user.cart().calculateTotal(catalog) * (1 - user.discount()) << '\n';
    cout << "Complete purchase? (yes/no): ";
    string answer;
    cin >> answer;
    if (answer != "yes" && answer != "YES") { cout << "Purchase canceled.\n"; return; }
    double total = 0;
    auto items = user.cart().items();
    if (!catalog.purchase(items, user.discount(), total)) {
        cout << "Purchase failed: product unavailable or insufficient stock.\n";
        return;
    }
    cout << "\nORDER CONFIRMED\nCustomer: " << user.name()
        << "\nMembership: " << user.membership()
        << "\nShip to: " << user.address() << '\n';
    for (const auto& item : items) cout << "Product ID " << item.first << " x" << item.second << '\n';
    cout << "Total charged (simulation): $" << fixed << setprecision(2) << total << '\n';
    user.cart().clear();
}

void concurrentDemo() {
    cout << "\n--- MULTITHREADING DEMO ---\n";
    Catalog shared;
    shared.addProduct(99, "Limited Edition Headphones", "Demo product", 49.99, 1);
    mutex outputMutex;
    auto shopper = [&](const string& name) {
        double total = 0;
        bool success = shared.purchase({ {99, 1} }, 0, total);
        lock_guard<mutex> outputLock(outputMutex);
        cout << name << (success ? " successfully purchased the last item.\n" : " could not purchase: out of stock.\n");
        };
    thread first(shopper, "Customer A");
    thread second(shopper, "Customer B");
    first.join();
    second.join();
    shared.display();
    cout << "Only one purchase succeeds. Inventory cannot go below zero.\n";
}

int main() {
    Catalog catalog;
    catalog.addProduct(1, "Wireless Mouse", "USB wireless mouse", 19.99, 10);
    catalog.addProduct(2, "Keyboard", "Mechanical keyboard", 59.99, 5);
    catalog.addProduct(3, "Headphones", "Noise-reducing headphones", 39.99, 8);

    cout << "WELCOME TO SHOPSMART\nEnter your name: ";
    string name, email, address;
    getline(cin, name);
    cout << "Email: "; getline(cin, email);
    cout << "Shipping address: "; getline(cin, address);
    while (name.empty() || email.find('@') == string::npos || address.empty()) {
        cout << "Please provide a name, valid-looking email, and address.\n";
        cout << "Name: "; getline(cin, name);
        cout << "Email: "; getline(cin, email);
        cout << "Shipping address: "; getline(cin, address);
    }
    int type;
    do { type = readInt("Membership (1 Regular, 2 Premium, 3 VIP): "); } while (type < 1 || type > 3);
    unique_ptr<User> user;
    if (type == 2) user = make_unique<PremiumUser>(name, email, address);
    else if (type == 3) user = make_unique<VIPUser>(name, email, address);
    else user = make_unique<User>(name, email, address);

    int choice = -1;
    while (choice != 0) {
        cout << "\n1 Browse products\n2 Add to cart\n3 Remove from cart\n4 Update cart quantity\n"
            << "5 View cart\n6 Checkout\n7 Multithreading demo\n"
            << "8 Add catalog product (demo admin)\n9 Update catalog product (demo admin)\n"
            << "10 Remove catalog product (demo admin)\n0 Exit\n";
        choice = readInt("Choose: ");
        if (choice == 1) catalog.display();
        else if (choice == 2) {
            int id = readInt("Product ID: "), quantity = readInt("Quantity: ");
            Product p(0, "", "", 0, 0);
            if (!catalog.getProduct(id, p) || quantity <= 0 || quantity > p.stock())
                cout << "Invalid product or quantity exceeds available stock.\n";
            else cout << (user->cart().add(id, quantity) ? "Added to cart (stock reserved only at checkout).\n" : "Invalid quantity.\n");
        }
        else if (choice == 3) {
            cout << (user->cart().remove(readInt("Product ID: ")) ? "Removed.\n" : "Item not in cart.\n");
        }
        else if (choice == 4) {
            int id = readInt("Product ID: "), quantity = readInt("New quantity (0 removes): ");
            cout << (user->cart().updateQuantity(id, quantity) ? "Cart updated.\n" : "Invalid quantity or item.\n");
        }
        else if (choice == 5) user->cart().display(catalog);
        else if (choice == 6) checkout(*user, catalog);
        else if (choice == 7) concurrentDemo();
        else if (choice == 8) {
            int id = readInt("New product ID: ");
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            string productName, description;
            cout << "Name: "; getline(cin, productName);
            cout << "Description: "; getline(cin, description);
            int cents = readInt("Price in cents (e.g., 1999): ");
            int stock = readInt("Stock: ");
            cout << (cents >= 0 && catalog.addProduct(id, productName, description, cents / 100.0, stock) ? "Product added.\n" : "Could not add product.\n");
        }
        else if (choice == 9) {
            int id = readInt("Product ID to update: ");
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            string productName, description;
            cout << "New name: "; getline(cin, productName);
            cout << "New description: "; getline(cin, description);
            int cents = readInt("New price in cents: ");
            int stock = readInt("New stock: ");
            cout << (cents >= 0 && catalog.updateProduct(id, productName, description, cents / 100.0, stock) ? "Updated.\n" : "Update failed.\n");
        }
        else if (choice == 10) {
            cout << (catalog.removeProduct(readInt("Product ID: ")) ? "Removed from catalog.\n" : "Product not found.\n");
        }
        else if (choice != 0) cout << "Invalid menu option.\n";
    }
    cout << "Thanks for using ShopSmart!\n";
}
