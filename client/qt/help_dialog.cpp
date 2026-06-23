#include "help_dialog.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

HelpDialog::HelpDialog(QWidget* parent):
    QDialog(parent),
    content(new QTextBrowser(this)),
    close_button(new QPushButton(tr("Cerrar"), this)) {

    setWindowTitle(tr("¿Cómo se juega?"));
    resize(680, 640);
    setModal(true);

    content->setOpenExternalLinks(false);
    content->setStyleSheet(
        "QTextBrowser {"
        "  background-color: #2a1f17;"
        "  color: #f0c870;"
        "  border: 2px solid #8a5a2b;"
        "  padding: 8px;"
        "  font-size: 13px;"
        "}");

    // Contenido en HTML para tener formato lindo (títulos, listas, énfasis).
    content->setHtml(tr(R"(
<h2 style="color: #f0c870;">Argentum Online — G5</h2>
<p>Bienvenida/o al juego. Acá te dejamos una guía rápida para empezar.</p>

<h3 style="color: #c89b3c;">Conectarte al servidor</h3>
<ol>
  <li>Asegurate de que el servidor esté corriendo en el host/puerto que vas a usar.</li>
  <li>Escribí tu <b>nick</b> en el campo central de la pantalla de bienvenida.</li>
  <li>Si necesitás cambiar host o puerto, expandí <i>Opciones de conexión</i>
      (por defecto: <code>127.0.0.1:8080</code>).</li>
  <li>Hacé clic en <b>ENTRAR</b>.</li>
</ol>

<h3 style="color: #c89b3c;">Lobby de partidas</h3>
<ul>
  <li><b>Unirse</b> a una partida existente: seleccioná una fila de la lista y
      hacé clic en <i>Unirse</i>.</li>
  <li><b>Crear</b> una partida: escribí un nombre, definí el máximo de jugadores
      (entre 2 y 8) y hacé clic en <i>Crear partida</i>.</li>
  <li>Usá <b>Refrescar</b> para actualizar la lista.</li>
  <li><b>Cerrar sesión</b> te devuelve a la pantalla de bienvenida.</li>
</ul>

<h3 style="color: #c89b3c;">Creación de personaje</h3>
<p>Después de unirte a una partida vas a elegir:</p>
<ul>
  <li><b>Raza:</b> Humano, Elfo, Enano o Gnomo.</li>
  <li><b>Clase:</b> Mago, Clérigo, Paladín o Guerrero.</li>
</ul>
<p>Cada combinación tiene stats iniciales distintos (HP, MP, ataque, defensa).
   Hacé clic en <i>Confirmar</i> y tu personaje aparece en el mundo.</p>

<h3 style="color: #c89b3c;">Controles en el juego</h3>
<ul>
  <li><b>Movimiento:</b> flechas direccionales (<code>&larr; &uarr; &darr; &rarr;</code>).</li>
  <li><b>Atacar:</b> click izquierdo sobre el objetivo.</li>
  <li><b>Abrir chat / comandos:</b> <code>Enter</code>, escribís el mensaje o comando, y otra vez
      <code>Enter</code> para enviarlo.</li>
</ul>
<p>Todos los <b>comandos especiales</b> se escriben en el chat empezando con <code>/</code>.
   Cualquier mensaje sin <code>/</code> es chat normal a todo el match.</p>

<h3 style="color: #c89b3c;">Comandos</h3>
<ul>
  <li><code>/tomar</code> — recoge el ítem del suelo donde estás parado y lo guarda en tu inventario.</li>
  <li><code>/tirar</code> — tira al piso el ítem seleccionado de tu inventario.</li>
  <li><b>Click izquierdo</b> sobre ítem en inventario: equipa el ítem, o lo usa
      de inmediato si es consumible (las pociones se consumen al instante).</li>
  <li><b>Click derecho</b> sobre ítem en inventario: selecciona el ítem para <code>/tirar</code>
      (se marca con borde celeste).</li>
  <li><code>/meditar</code> — entrás en estado de meditación y recuperás maná con el tiempo.
      Requiere estar quieto; cualquier otra acción te saca de este estado.</li>
  <li><code>/resucitar</code> — te transporta al sacerdote y te devuelve la vida (HP al máximo).
      Quedás inmovilizado un tiempo proporcional a tu distancia a él.</li>
</ul>

<h3 style="color: #c89b3c;">Interacciones con NPCs</h3>
<p>Para interactuar con un NPC: click sobre él (sacerdote, comerciante o banquero) y usá los
   comandos correspondientes.</p>

<p style="color: #f0c870; margin-bottom: 4px;"><b>Sacerdote</b></p>
<ul>
  <li><code>/curar</code> — te restaura HP y maná al máximo.</li>
  <li><code>/resucitar</code> — te devuelve a la vida.</li>
  <li><code>/comprar &lt;objeto&gt;</code> — comprás pociones o hechizos (varas y báculos).
      Nunca vende armas ni armaduras.</li>
  <li><code>/listar</code> — muestra el catálogo de objetos que tiene para vender.</li>
</ul>

<p style="color: #f0c870; margin-bottom: 4px;"><b>Comerciante</b></p>
<ul>
  <li><code>/comprar &lt;objeto&gt;</code> — compra un arma, armadura, casco u otro objeto.</li>
  <li><code>/vender &lt;objeto&gt;</code> — vende un objeto de tu inventario al comerciante.</li>
  <li><code>/listar</code> — muestra el catálogo de objetos disponibles.</li>
</ul>

<p style="color: #f0c870; margin-bottom: 4px;"><b>Banquero</b></p>
<ul>
  <li><code>/depositar &lt;objeto&gt;</code> — guarda un objeto de tu inventario en el banco.</li>
  <li><code>/depositar oro &lt;cantidad&gt;</code> — guarda oro en el banco.</li>
  <li><code>/retirar &lt;objeto&gt;</code> — recupera un objeto del banco a tu inventario.</li>
  <li><code>/retirar oro &lt;cantidad&gt;</code> — retira oro del banco.</li>
  <li><code>/listar</code> — muestra los objetos y el oro guardados.</li>
</ul>
<p>El sistema bancario es <b>global</b>: podés depositar en una ciudad y retirar desde cualquier
   otra sucursal.</p>

<h3 style="color: #c89b3c;">Combate</h3>
<ul>
  <li>Ganás <b>experiencia</b> al atacar tanto a jugadores como a NPCs hostiles.
      Subir de nivel mejora tus stats automáticamente.</li>
  <li>Si tu <b>HP llega a 0</b> te convertís en fantasma y perdés oro y experiencia;
      todos tus objetos caen al piso.</li>
  <li>Como <b>fantasma</b> podés moverte, pero no interactuar con nada ni nadie.</li>
  <li>Para revivir: usá <code>/resucitar</code> o acercate a un sacerdote.</li>
</ul>

<h3 style="color: #c89b3c;">Chat y mensajería</h3>
<ul>
  <li><code>@&lt;nick&gt; &lt;mensaje&gt;</code> — envía un mensaje privado a otro jugador.</li>
  <li>El chat es global a tu partida: lo ven todos los jugadores conectados a ese match.</li>
  <li>Si el servidor se cae, el cliente muestra <i>"Desconectado"</i> y vuelve a la pantalla de bienvenida.</li>
</ul>

<h3 style="color: #c89b3c;">Clanes</h3>
<ul>
  <li><code>/fundar-clan &lt;nombre&gt;</code> — funda un clan nuevo (requiere nivel mínimo).
      No podés irte ni fundar otro después.</li>
  <li><code>/unirse &lt;nombre del clan&gt;</code> — pide unirte a un clan existente.</li>
  <li><code>/revisar-clan</code> — muestra miembros actuales y pedidos pendientes (solo el fundador).</li>
  <li><code>/clan-aceptar &lt;nick&gt;</code> — acepta a un jugador que pidió unirse (solo el fundador).</li>
  <li><code>/clan-rechazar &lt;nick&gt;</code> — rechaza un pedido de ingreso (solo el fundador).</li>
  <li><code>/clan-ban &lt;nick&gt;</code> — rechaza y además bloquea futuros pedidos de ese jugador (solo el fundador).</li>
  <li><code>/clan-kick &lt;nick&gt;</code> — expulsa a un jugador del clan, sin banearlo (solo el fundador).</li>
  <li><code>/dejar-clan</code> — salís del clan. El fundador no puede usar este comando.</li>
</ul>

<h3 style="color: #c89b3c;">Cheats útiles</h3>
<ul>
  <li><code>Ctrl+1</code> — HP al máximo</li>
  <li><code>Ctrl+2</code> — MP al máximo</li>
  <li><code>Ctrl+3</code> — Morir instantáneamente</li>
  <li><code>Ctrl+4</code> — Subir de nivel</li>
  <li><code>Ctrl+5</code> — Subir oro al máximo</li>
</ul>

<p style="color: #8a5a2b; margin-top: 20px;">
<i>Argentum Online TP — Taller 1, Cátedra Veiga, 1c 2026.</i></p>
)"));

    close_button->setMinimumHeight(36);
    close_button->setDefault(true);

    auto* button_row = new QHBoxLayout;
    button_row->addStretch();
    button_row->addWidget(close_button);

    auto* root = new QVBoxLayout(this);
    root->addWidget(content);
    root->addLayout(button_row);
    root->setContentsMargins(12, 12, 12, 12);

    connect(close_button, &QPushButton::clicked, this, &HelpDialog::accept);
}
