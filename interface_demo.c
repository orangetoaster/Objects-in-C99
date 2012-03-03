/** This is a demonstration of the core OOP principles in ansi c99.
 * Copyright (C) 2012 Lorne Schell <orange.toaster@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * The core OOP principles are:
 *  Encapsulation:
 *   Encapsulation is used to restict access to internal workings of some subsystem.
 *   This is to allow a particular subset of code to be changed as needed, and as long as it maintains it's public function contracts then the system in otherwise unaffected.
 *   C allows encapsulation by only declaring what is public in the included .h file, and keeping what is private unannounced in the .c file.
 *   This prevents other programmers from accidentally using private members.
 *   C offers even more, if the .c files are not provided and a library is provided instead, then the .c contents are not even known; providing binary encapsulation.
 *   C++ does not have binary encapsulation, Java does not have runtime encapsulation (reflection). Python does not have any encapsulation.
 *  Delegation:
 *   Delegation is used to simplify the current task by letting some subsystem or shared subset of code do the work for you.
 *   This allows the DRY principle to be upheld.
 *   C++ and Java allow the use of inheritance for delegation, by delegating to methods overridden by a child, disallowing dependancy injection.
 *   C allows the simplist and most powerful form of delegation, through member variables; this allows polymorphic dependancy injection without reflection.
 *  Polymorphism:
 *   Polymorphism is used to declare a supported interface, or functional contract, and then allow anything that supports that interface to be used dynamically.
 *   Java uses interfaces, C++ uses virtual parents, go and ruby use ducktyping. 
 *   C can consisely and efficiently do all of them.
 * Hence, C is the ideal object-oriented language.
 */
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

// Object System - declare publicly in object.h //
typedef struct iface {
    size_t vtable_size;
} iface;

typedef struct impl {
    iface * type;
    void * vtable;
} impl;

typedef struct class {
    size_t instance_size;
    unsigned char interface_count; // 256 ifaces is reasonable
    impl *interfaces;
} class;

typedef struct object {
    struct class * type;
    void * instance;
} object;

impl has_iface(object self, iface *interface) {
    unsigned char i=0;
    for(i=0; i < self.type->interface_count; ++i) {
        if(self.type->interfaces[i].type == interface) return self.type->interfaces[i];
    }
    return (impl) { .type = NULL, .vtable = NULL };
}

int iface_isnull(impl interface) {
    return interface.vtable == NULL;
}

// Polymorphic Printer - declare publicly in println.h //
typedef struct printable_vtable {
    void (*toString) (object self, size_t length, char * buf);
} printable_vtable;

iface printable_iface = { .vtable_size = sizeof(printable_vtable) };

void println(object self, impl contract);

// Polymorphic Printer - encapsulate privately in println.c //
void println(object self, impl contract) {
    assert(contract.type = &printable_iface);
    printable_vtable *v = (printable_vtable *) contract.vtable;
    char buf[] = { '\0', '\n' };
    v->toString(self, 2, buf);
    write(1, buf, 2);
}

// Number Contract - declare publicly in number.h //
class number_class;
impl number_printable_impl;
printable_vtable number_printable_vtable;

object new_number(int value);
void destroy_number(object * self);
impl new_runtime_number_print_iface(object _self);
void destroy_number_print_iface(impl *generated);

// Number Object - encapsulate privately in number.c //
class number_class;
struct number {
    int representation;
};

object new_number(int value) {
    struct number * self = malloc( sizeof(struct number) );
    self->representation = value;
    return (object) { .type = &number_class, .instance = self };
}

void destroy_number(object * self) {
    free(self->instance);
    self->instance = 0;
}

// note that this is publicly accessible but not publicly declared.
void number_print ( object _self, size_t length, char * buf ) { 
    assert(_self.type == &number_class);
    struct number * self = (struct number *) _self.instance;
    if(length > 1) {
        *buf = self->representation + '0';
    }
}

printable_vtable number_printable_vtable = {
    .toString = &number_print
};

impl new_runtime_number_print_iface(object _self) {
    assert(_self.type == &number_class);
    printable_vtable * vtable = malloc( sizeof(printable_vtable) );
    vtable->toString = &number_print;
    return (impl) { .type = &printable_iface, .vtable = vtable };
}

void destroy_number_print_iface(impl *generated) {
    free(generated->vtable);
    generated->vtable = 0;
}

impl number_printable_impl = { .type = &printable_iface, .vtable = &number_printable_vtable };
static impl number_interfaces[] = {
    { .type = &printable_iface, .vtable = &number_printable_vtable }
};

class number_class = {
    .instance_size = sizeof(struct number),
    .interface_count = sizeof(number_interfaces) / sizeof(void *),
    .interfaces = number_interfaces
};

// Example Usage //
int main() {
    object num = new_number(3);
    // runtime class-generated vtable - java style (except java leaves it for the gc to collect) //
    impl generated = new_runtime_number_print_iface(num);
    println(num, generated);
    destroy_number_print_iface(&generated);
    // runtime call-generated vtable - ducktyping style (ruby, php, go), also like using reflection in java //
    printable_vtable my_num_vtable = {
        .toString = number_printable_vtable.toString
    };
    impl my_printable_implementation = {
        .type = &printable_iface,
        .vtable = &my_num_vtable
    };
    println(num, my_printable_implementation);
    // compiletime vtable - C++ style //
    println(num, number_printable_impl);
    // message passing - objective-c style //
    impl contract;
    if(!iface_isnull(contract = has_iface(num, &printable_iface))) {
        println(num, contract);
    }
    destroy_number(&num);
    return 0;
}
