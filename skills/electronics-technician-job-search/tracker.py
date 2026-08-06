import csv
import os

FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "applications.csv")

def load_applications():
    if not os.path.exists(FILE):
        return []
    with open(FILE, newline="") as f:
        return list(csv.DictReader(f))

def save_applications(applications):
    with open(FILE, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["company", "title", "date", "status"])
        writer.writeheader()
        writer.writerows(applications)

def print_applications(applications):
    print("\n--- Your Applications ---")
    for app in applications:
        print(f"{app['date']} | {app['company']} - {app['title']} ({app['status']})")

applications = load_applications()

while True:
    action = input("\n(a)dd new, (u)pdate status, (f)ilter, (x) delete, (d)one: ").lower()

    if action == "d":
        break

    elif action == "a":
        company = input("Company name: ")
        title = input("Job title: ")
        date = input("Date applied (e.g. 2026-08-03): ")
        status = input("Status (applied/interview/rejected): ")
        applications.append({
            "company": company,
            "title": title,
            "date": date,
            "status": status
        })
        save_applications(applications)

    elif action == "u":
        company = input("Which company do you want to update? ")
        found = False
        for app in applications:
            if app["company"].lower() == company.lower():
                app["status"] = input(f"New status for {app['company']}: ")
                found = True
        if not found:
            print(f"No entry found for '{company}'.")
        save_applications(applications)

    elif action == "f":
        wanted_status = input("Show entries with status: ").lower()
        matches = [app for app in applications if app["status"].lower() == wanted_status]
        if matches:
            print_applications(matches)
        else:
            print(f"No entries with status '{wanted_status}'.")

    elif action == "x":
        company = input("Which company do you want to delete? ")
        before_count = len(applications)
        applications = [app for app in applications if app["company"].lower() != company.lower()]
        if len(applications) < before_count:
            print(f"Deleted entry for '{company}'.")
        else:
            print(f"No entry found for '{company}'.")
        save_applications(applications)

print_applications(applications)
