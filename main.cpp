#include "headers/lojaGerencia.hpp"
#include <iostream>

using namespace std;

int main() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        PGconn* conn = conectar();
        if (conn == nullptr)
                return 1;

        Loja loja("Loja Musical", "00.000.000/0001-00",
                  "Rua das Guitarras, 123", "(11) 99999-9999");

        cout << "Sistema iniciado com sucesso!" << endl;
        loja.exibir();

        loja.menu(conn);

        PQfinish(conn);
        return 0;
}