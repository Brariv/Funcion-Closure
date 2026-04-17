#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <queue>
#include <sstream>
#include <algorithm>

using namespace std;

// ---------- Estructuras ----------

struct Produccion {
    string cabeza;
    vector<string> cuerpo;
};

struct Item {
    string cabeza;
    vector<string> cuerpo;
    int punto;

    bool operator<(const Item& o) const {
        if (cabeza != o.cabeza) return cabeza < o.cabeza;
        if (cuerpo != o.cuerpo) return cuerpo < o.cuerpo;
        return punto < o.punto;
    }
    bool operator==(const Item& o) const {
        return cabeza == o.cabeza && cuerpo == o.cuerpo && punto == o.punto;
    }
};

// ---------- Variables globales ----------
vector<Produccion> gramatica;
set<string> noTerminales;
string simboloInicial;          // cabeza de la primera produccion

// ---------- Utilidades ----------

vector<string> tokenizar(const string& s) {
    vector<string> toks;
    stringstream ss(s);
    string t;
    while (ss >> t) toks.push_back(t);
    return toks;
}

string itemToString(const Item& it) {
    string s = it.cabeza + " -> ";
    if (it.cuerpo.empty()) { s += "."; return s; }
    for (int i = 0; i < (int)it.cuerpo.size(); ++i) {
        if (i == it.punto) s += ". ";
        s += it.cuerpo[i];
        if (i + 1 < (int)it.cuerpo.size()) s += " ";
    }
    if (it.punto == (int)it.cuerpo.size()) s += " .";
    return s;
}

bool esNoTerminal(const string& sim) {
    return noTerminales.count(sim) > 0;
}

// ---------- Lectura de la gramatica ----------

void leerGramatica() {
    cout << "Ingrese las producciones de la gramatica, una por linea.\n";
    cout << "La PRIMERA produccion define el simbolo inicial aumentado\n";
    cout << "(por ejemplo:  S' -> E).\n";
    cout << "Formato:  S -> S L ...   ('eps' para epsilon)\n";
    cout << "Escriba una linea vacia para terminar.\n\n";

    string linea;
    while (true) {
        cout << "> ";
        if (!getline(cin, linea)) break;
        if (linea.empty()) break;

        size_t pos = linea.find("->");
        if (pos == string::npos) {
            cout << "  (formato invalido)\n";
            continue;
        }

        string izq = linea.substr(0, pos);
        string der = linea.substr(pos + 2);

        vector<string> cabezaToks = tokenizar(izq);
        if (cabezaToks.size() != 1) {
            cout << "  (la cabeza debe ser un solo simbolo)\n";
            continue;
        }

        Produccion p;
        p.cabeza = cabezaToks[0];
        p.cuerpo = tokenizar(der);

        if (p.cuerpo.size() == 1 && (p.cuerpo[0] == "eps" || p.cuerpo[0] == "epsilon")) {
            p.cuerpo.clear();
        }

        gramatica.push_back(p);
        noTerminales.insert(p.cabeza);
    }

    for (size_t i = 0; i < gramatica.size(); ++i) {
        vector<string>& cuerpo = gramatica[i].cuerpo;
        while (true) {
            auto it = find(cuerpo.begin(), cuerpo.end(), "|");
            if (it == cuerpo.end()) break;
            
            vector<string> beta(it + 1, cuerpo.end());
            cuerpo.erase(it, cuerpo.end());
            
            Produccion alt = { gramatica[i].cabeza, beta };
            gramatica.push_back(alt);
        }
    }

    if (!gramatica.empty()) {
        simboloInicial = gramatica[0].cabeza;
    }

    cout << "\nGramatica leida (" << gramatica.size() << " producciones):\n";
    for (size_t i = 0; i < gramatica.size(); ++i) {
        cout << "  (" << i + 1 << ") " << gramatica[i].cabeza << " -> ";
        if (gramatica[i].cuerpo.empty()) cout << "eps";
        else for (auto& s : gramatica[i].cuerpo) cout << s << " ";
        cout << "\n";
    }
    cout << "Simbolo inicial (aumentado): " << simboloInicial << "\n\n";
}

// ---------- Funcion CERRADURA ----------

// verbose = true imprime la traza paso a paso.
set<Item> CERRADURA(const set<Item>& inicial, bool verbose) {
    set<Item> J;
    for (auto& it : inicial) J.insert(it);

    if (verbose) {
        cout << "  Items de entrada:\n";
        for (auto& it : inicial) cout << "    " << itemToString(it) << "\n";
    }

    bool seAgrego = true;
    int iteracion = 0;

    while (seAgrego) {
        seAgrego = false;
        iteracion++;
        bool algoEnIter = false;
        if (verbose) cout << "  Iteracion " << iteracion << ":\n";

        vector<Item> actuales(J.begin(), J.end());

        for (auto& it : actuales) {
            if (it.punto < (int)it.cuerpo.size()) {
                string B = it.cuerpo[it.punto];
                if (esNoTerminal(B)) {
                    for (auto& p : gramatica) {
                        if (p.cabeza == B) {
                            Item nuevo{ p.cabeza, p.cuerpo, 0 };
                            if (J.insert(nuevo).second) {
                                if (verbose) {
                                    cout << "    + " << itemToString(nuevo)
                                         << "   (por " << itemToString(it) << ")\n";
                                }
                                seAgrego = true;
                                algoEnIter = true;
                            }
                        }
                    }
                }
            }
        }

        if (verbose && !algoEnIter) {
            cout << "    (no se agregaron nuevos items) -> fin\n";
        }
    }

    return J;
}


// Obtiene todos los simbolos (terminales y no-terminales) que
// aparecen justo despues del punto en algun item del conjunto.
vector<string> simbolosDespuesDelPunto(const set<Item>& I) {
    set<string> s;
    for (auto& it : I) {
        if (it.punto < (int)it.cuerpo.size()) {
            s.insert(it.cuerpo[it.punto]);
        }
    }
    return vector<string>(s.begin(), s.end());
}

// ---------- Impresion de una cerradura ----------
void imprimirCerradura(int idx, const set<Item>& I) {
    cout << "  I" << idx << " (" << I.size() << " items):\n";
    int n = 1;
    for (auto& it : I) {
        cout << "    " << n++ << ") " << itemToString(it) << "\n";
    }
}

// ---------- Programa principal ----------

int main() {
    cout << "  Calculo de funcion de cerradura LR(0)\n";

    leerGramatica();
    if (gramatica.empty()) {
        cout << "No se ingresaron producciones. Fin.\n";
        return 0;
    }

    // --- Construir el item inicial: S' -> . alfa  (primera produccion) ---
    // Ejemplo: S' -> . S
    Item itemInicial;
    itemInicial.cabeza = gramatica[0].cabeza + "'";
    itemInicial.cuerpo = {gramatica[0].cabeza};
    itemInicial.punto  = 0;

    cout << "Item inicial tomado de la primera produccion:\n";
    cout << "  " << itemToString(itemInicial) << "\n\n";

    // --- Lista de estados (cerraduras) y cola para BFS ---
    vector<set<Item>> estados;                 // estados[i] = Ii
    map<set<Item>, int> indice;                // cerradura -> numero de estado
    // Transiciones: (estado_origen, simbolo) -> estado_destino
    vector<map<string,int>> trans;

    // --- I0: cerradura del item inicial ---
    cout << " Calculando CERRADURA de I0\n";
    set<Item> I0 = CERRADURA({itemInicial}, true);
    estados.push_back(I0);
    indice[I0] = 0;
    trans.push_back({});
    cout << "\n";
    imprimirCerradura(0, I0);
    cout << "\n";

    // --- BFS sobre todos los estados ---
    queue<int> cola;
    cola.push(0);

    while (!cola.empty()) {
        int i = cola.front(); cola.pop();
        // Nota: tomamos una COPIA (no referencia) porque mas abajo
        // hacemos push_back a `estados`, lo cual puede invalidar
        // cualquier referencia al vector.
        set<Item> Ii = estados[i];

        vector<string> simbolos = simbolosDespuesDelPunto(Ii);
        for (auto& X : simbolos) {
            // Calcular la base del nuevo estado (solo para mostrarlo)
            set<Item> base;
            for (auto& it : Ii) {
                if (it.punto < (int)it.cuerpo.size() && it.cuerpo[it.punto] == X) {
                    Item mov = it;
                    mov.punto++;
                    base.insert(mov);
                }
            }
            if (base.empty()) continue;

            // ¿Ya conocemos la cerradura resultante?
            set<Item> J = CERRADURA(base, false);  // sin traza
            auto it = indice.find(J);
            int destino;
            if (it == indice.end()) {
                destino = (int)estados.size();
                estados.push_back(J);
                indice[J] = destino;
                trans.push_back({});

                // Imprimir con traza la cerradura nueva
                cout << " Calculando CERRADURA de I" << destino
                     << "  =  ir_a(I" << i << ", " << X << ")\n";
                // Volver a calcular con verbose para mostrar traza
                CERRADURA(base, true);
                cout << "\n";
                imprimirCerradura(destino, J);
                cout << "\n";

                cola.push(destino);
            } else {
                destino = it->second;
            }
            trans[i][X] = destino;
        }
    }

    // --- Resumen final ---
    cout << "  RESUMEN: Todas las cerraduras (" << estados.size() << " estados)\n";
    for (int i = 0; i < (int)estados.size(); ++i) {
        imprimirCerradura(i, estados[i]);
        cout << "\n";
    }

    return 0;
}
