# ShopSmart: C++ Online Shopping System

#### Video Demo: (https://youtu.be/6xqTXmXgYR4)

#### Description:

ShopSmart is a console-based online shopping application that I developed as a final project to demonstrate object-oriented programming, input validation, simulated purchasing, and multithreading in C++. I wanted to create a project that is realistic enough to represent some of the main tasks performed by an online store while still being understandable for someone who is continuing to learn C++. The application runs in a terminal, so users interact with a numbered menu rather than a website. It is intended for educational use and does not process real payments or store actual customer accounts.

When the application starts, the customer enters a name, email address, and shipping address. The customer then chooses one of three membership types: Regular, Premium, or VIP. Regular members do not receive a discount, Premium members receive a 10% discount, and VIP members receive a 20% discount. These discounts are examples that I selected for the project. The customer can browse products, add products to a shopping cart, remove items, update quantities, view the cart, and check out. The program includes three sample products so the user can start testing immediately. It also provides demonstration options for adding, updating, and removing catalog products.

## Project Files

**main.cpp** contains the complete C++ application. I chose to keep the classes in one file because this is a small educational project and it makes the relationships easier to follow. This file defines the Product, Catalog, ShoppingCart, User, PremiumUser, and VIPUser classes. It also contains the menu, input-handling functions, checkout simulation, and multithreading demonstration. In a larger application, I would separate these classes into individual header and implementation files to make maintenance easier.

**README.md** is the documentation file. It explains the purpose of the project, its features, how to compile and run the program, and the reasoning behind important design decisions. It also contains the link to the project demonstration video once I upload it.

## Design Decisions

I used object-oriented programming because it helps divide a larger problem into smaller responsibilities. The Product class stores product information such as an identifier, name, description, price, and available stock. The Catalog class stores the collection of products and provides methods for adding, updating, and removing products. I separated the Catalog from the Product class because an individual product should not have to manage the entire product collection. The ShoppingCart class keeps track of product identifiers and quantities for one customer. This design also allows the catalog to remain the source of current product prices and stock quantities.

The User class stores the customer's name, email, shipping address, and cart. PremiumUser and VIPUser inherit from User and override virtual methods that define membership discounts. I used inheritance and polymorphism to avoid writing completely separate checkout logic for each customer type. The program can calculate a discount through the same User interface, even though different customer types receive different rates. Most class attributes are private, which demonstrates encapsulation by requiring other parts of the program to use public methods rather than changing data directly.

The checkout process is simulated. The program asks the customer to type yes before completing the order. After confirmation, it checks whether all requested products still exist and whether enough stock is available. If the order is valid, the application reduces the stock quantities and prints an order confirmation with the customer's information and total. If an item is unavailable, the purchase is rejected without partially reducing stock. This is important because adding an item to a cart does not guarantee it will still be in stock at checkout. In a real store, checkout would also require a secure payment provider and a database transaction.

## Multithreading and Thread Safety

The multithreading demonstration creates two C++ threads that represent customers attempting to purchase the same limited-stock product. Only one unit is available. Both threads call the same catalog purchase method. I used a std::mutex with std::lock_guard to protect shared catalog data. The lock ensures that checking stock and reducing stock happen together, so only one thread can successfully purchase the item. The other thread receives an out-of-stock result. This demonstrates a simple solution to a race condition. A separate mutex protects console output so the two thread messages do not overlap. I used join() to wait for both threads to finish before displaying the remaining inventory.

## Input Validation and Limitations

The program checks that menu selections and quantities are whole numbers. It rejects invalid product identifiers, negative stock, and negative prices. It also performs a basic check that an email contains the @ symbol. These checks are useful for a class project, although a production application would require stronger validation. The program uses double for prices for simplicity; a production financial system should use integer cents or an appropriate decimal representation to avoid rounding problems. Product data and orders are not saved after the program exits. Catalog management options are labeled as demo administration tools and are not protected by a real administrator login, so this application should not be treated as a secure commercial store.

## How to Compile and Run

A C++17-compatible compiler is required. From a terminal in the project folder, run:

```bash
g++ -std=c++17 -pthread main.cpp -o shopsmart
./shopsmart
```

On Windows, the executable may be named shopsmart.exe, and it can be started using `./shopsmart.exe` or `shopsmart.exe`, depending on the terminal. Start by browsing the catalog, adding a product to the cart, and completing a simulated purchase. Then choose the multithreading demo from the menu to see two customers compete for the final unit of a product. Finally, test an invalid menu choice or an out-of-stock purchase to see how the application responds.

## Reflection

This project helped me understand how individual C++ topics connect in a working program. Classes made it easier to organize products, customers, and carts. Inheritance allowed me to represent membership types without duplicating all of the user information. The checkout process required me to think carefully about validation and inventory updates. Multithreading was the most interesting challenge because two operations can interfere with one another when they share data. By using mutexes and testing a limited-stock purchase, I was able to demonstrate why synchronization matters. If I continued developing ShopSmart, I would add persistent storage, real user authentication, order history, stronger price handling, and more realistic payment processing.
