#include "CapriceVKeyboard.h"
#include "CapriceVKeyboard.h"
#include "keyboard.h"
#include "cap32.h"

extern t_CPC CPC;

namespace wGui {

  CapriceVKeyboard::CapriceVKeyboard(const CRect& WindowRect, CWindow* pParent, SDL_Surface* screen, CFontEngine* pFontEngine) :
    CFrame(WindowRect, pParent, pFontEngine, "Caprice32 - Virtual Keyboard", false), m_pScreenSurface(screen)
  {
    SetModal(true);
    // TODO: TAB ?
    m_keyFromChar = keysFromChars[CPC.kbd_layout];
    std::vector<std::string> keys{ "ABCDEFGHIJ", "KLMNOPQRST", "UVWXYZabcd", "efghijklmn", "opqrstuvwx", "yz01234567", "89&#\"'(-_)", "=,.:!|?./*", "+%<>[]{}\\`"};
    // TODO: make this configurable
    std::vector<std::string> keywords{ "cat\n", "run\n", "run\"", "cls\n", "mode ", "|cpm\n", "|tape\n", "|a\n", "|b\n" };
    // TODO: add files that are on disk
    m_focused.first = 0;
    m_focused.second = 0;
    m_result = new CEditBox(CRect(CPoint(11, 15), 390, 15), this);
    m_result->SetReadOnly(true);
    // Add one char keys
    int y = 40;
    for(auto& l : keys)
    {
      std::vector<CButton*> line;
      int x = 10;
      for(auto& c : l)
      {
        CButton *button = new CButton(CRect(CPoint(x, y), 15, 15), this, std::string(1, c));
        button->SetIsFocusable(true);
        line.push_back(button);
        x += 20;
      }
      y += 20;
      m_buttons.push_back(line);
    }
    // Add keywords
    int x = 150;
    int nb_lines = m_buttons.size();
    int i = m_buttons.size();
    for(auto& w : keywords)
    {
      if(i >= nb_lines) {
        if(x > 300) break;
        i -= nb_lines;
        y = 40;
        x += 90;
      }
      CButton *button = new CButton(CRect(CPoint(x, y), 70, 15), this, w);
      button->SetIsFocusable(true);
      m_buttons[i++].push_back(button);
      y += 20;
    }
    // Add SPACE, DELETE and RETURN buttons
    std::vector<CButton*> line;
    CButton *space = new CButton(CRect(CPoint(10,  y), 55, 15), this, "SPACE");
    CButton *retur = new CButton(CRect(CPoint(80,  y), 55, 15), this, "RETURN");
    CButton *backs = new CButton(CRect(CPoint(150, y), 55, 15), this, "DELETE");
    space->SetIsFocusable(true);
    retur->SetIsFocusable(true);
    backs->SetIsFocusable(true);
    line.push_back(space);
    line.push_back(retur);
    line.push_back(backs);
    m_buttons.push_back(line);
  }

  CapriceVKeyboard::~CapriceVKeyboard() { };

  void CapriceVKeyboard::CloseFrame(void) {
    // Exit gui
    std::cout << "CloseFrame" << std::endl;
    CMessageServer::Instance().QueueMessage(new CMessage(CMessage::APP_EXIT, nullptr, this));
  }

  std::list<SDL_Event> CapriceVKeyboard::GetEvents() {
    std::list<SDL_Event> result;
    for(auto c : m_result->GetWindowText()) {
      SDL_Event key;
      // key.key.keysym.scancode = ;
      key.key.keysym.sym = m_keyFromChar[c].first;
      key.key.keysym.mod = m_keyFromChar[c].second;
      // key.key.keysym.unicode = c;

      key.key.type = SDL_KEYDOWN;
      key.key.state = SDL_PRESSED;
      result.push_back(key);

      key.key.type = SDL_KEYUP;
      key.key.state = SDL_RELEASED;
      result.push_back(key);
    }
    return result;
  }

  void CapriceVKeyboard::moveFocus(int dy, int dx) {
    m_buttons[m_focused.first][m_focused.second]->SetHasFocus(false);

    m_focused.first += dy;
    int height = m_buttons.size();
    if(m_focused.first < 0) m_focused.first += height;
    if(m_focused.first >= height) m_focused.first -= height;

    m_focused.second += dx;
    int width = m_buttons[m_focused.first].size();
    if(m_focused.second < 0) m_focused.second += width;
    if(m_focused.second >= width) {
      if(dx == 0) {
        m_focused.second = width - 1;
      } else {
        m_focused.second -= width;
      }
    }

    m_buttons[m_focused.first][m_focused.second]->SetHasFocus(true);
  }

  bool CapriceVKeyboard::HandleMessage(CMessage* pMessage)
  {
    bool bHandled = false;

    if (pMessage)
    {
      switch (pMessage->MessageType())
      {
        case CMessage::APP_DESTROY_FRAME:
          bHandled = true;
          CloseFrame();
          break;
        case CMessage::CTRL_SINGLELCLICK:
          if (pMessage->Destination() == this) {
            std::string pressed = static_cast<const CWindow*>(pMessage->Source())->GetWindowText();
            std::cout << "Pressed: " << pressed << std::endl;
            if(pressed == "SPACE") {
              pressed = " ";
            }
            else if(pressed == "RETURN") {
              pressed = "\n";
            }
            else if(pressed == "DELETE") {
              std::string result = m_result->GetWindowText();
              // If the string is not empty, and last char is not backspace remove it
              if(!result.empty() && result[result.size()-1] != '\b') {
                m_result->SetWindowText(result.substr(0, result.size()-1));
                break;
              }
              // Otherwise put backspace in the output
              pressed = "\b";
            }
            std::string result = m_result->GetWindowText() + pressed;
            m_result->SetWindowText(result);
          }
          break;
        case CMessage::KEYBOARD_KEYDOWN:
          if (m_bVisible && pMessage->Destination() == this) {
            CKeyboardMessage* pKeyboardMessage = dynamic_cast<CKeyboardMessage*>(pMessage);
            if (pKeyboardMessage) {
              switch (pKeyboardMessage->Key) {
                case SDLK_UP:
                  moveFocus(-1, 0);
                  bHandled = true;
                  break;
                case SDLK_DOWN:
                  moveFocus(1, 0);
                  bHandled = true;
                  break;
                case SDLK_LEFT:
                  moveFocus(0, -1);
                  bHandled = true;
                  break;
                case SDLK_RIGHT:
                  moveFocus(0, 1);
                  bHandled = true;
                  break;
                case SDLK_ESCAPE:
                  bHandled = true;
                  CloseFrame();
                  break;
                default:
                  break;
              }
            }
          }
          break;
        default:
          break;
      }
    }
    if(!bHandled) {
      bHandled = CFrame::HandleMessage(pMessage);
    }

    return bHandled;
  }
}
