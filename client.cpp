#include "tic.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <exception>
#include <fstream>
#include <limits>

#include <stdlib.h>


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 80
#define SERV_PORT 9999


//*****************************************************************************
// PlayerEnum

// PlayerEnum::PlayerEnum(const std::string& representation)
// {
    // if (representation == "X") m_player = X;
    // else if (representation == "O") m_player = O;
    // else m_player = Player::NONE;
// }

std::string PlayerEnum::toString(PlayerEnum::Player player)
{
  switch (player)
  {
  case X:
    return "X";
  case O:
    return "O";
  default:
    break;
  }
  return "-";
}

PlayerEnum::Player PlayerEnum::getOtherPlayer(PlayerEnum::Player player)
{
  switch (player)
  {
  case X:
    return O;
  case O:
    return X;
  default:
    break;
  }
  return NONE;
}

std::string pe(PlayerEnum::Player player)
{
    return PlayerEnum::toString(player);
}

//*****************************************************************************
// Field

Field::Field(int number, PlayerEnum::Player owner)
  : m_number(number)
  , m_owner(owner)
{
}

Field Field::copy() const
{
  return Field(m_number, m_owner);
}

PlayerEnum::Player Field::getOwner() const
{
  return m_owner;
}

void Field::setOwner(PlayerEnum::Player owner)
{
  m_owner = owner;
}

int Field::getNumber() const
{
  return m_number;
}

std::string Field::toString() const
{
  std::ostringstream s;
  s << "(" << std::setw(2) << m_number << ") " << PlayerEnum::toString(m_owner);
  return s.str();
}

//*****************************************************************************
// Board

Board::Board()
{
  initialize();
}

void Board::initialize()
{
  m_playingBoard.clear();
  m_playingBoard.push_back(Field());
  for (int row = 1; row <= ROW_DIMENSION; row++)
  {
    for (int col = 1; col <= COL_DIMENSION; col++)
    {
      Field field(number(row, col));
      m_playingBoard.push_back(field);
    }
  }
}

void Board::move(int number, PlayerEnum::Player owner)
{
  if (m_playingBoard[number].getOwner() != PlayerEnum::NONE)
  {
    std::cerr << "Illegal move, owner " << owner << " field already occupied: " << number << std::endl;
    printToScreen();
    throw std::runtime_error("Illegal move");
  }
  m_playingBoard[number].setOwner(owner);
}

Board Board::copy() const
{
  Board board;

  for (int row = 1; row <= ROW_DIMENSION; row++)
  {
    for (int col = 1; col <= COL_DIMENSION; col++)
    {
      Field f = m_playingBoard[number(row, col)];
      board.setField(f.getNumber(), f.copy());
    }
  }
  return board;
}

std::vector<int> Board::validMoves() const
{
  std::vector<int> validMoves;

  for (int row = 1; row <= ROW_DIMENSION; row++)
  {
    for (int col = 1; col <= COL_DIMENSION; col++)
    {
      if (isValid(row, col))
      {
        validMoves.push_back(number(row, col));
      }
    }
  }
  return validMoves;
}

bool Board::isValid(int row, int col) const
{
  if (col == -999)
  {
    int n = row;
    if (n > (ROW_DIMENSION * COL_DIMENSION))
    {
      return false;
    }
    else if (m_playingBoard[n].getOwner() == PlayerEnum::NONE)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return isValid(number(row, col));
  }
}

bool Board::isWon(PlayerEnum::Player player) const
{
  for (int row = 1; row <= ROW_DIMENSION; row++)
  {
    int count = 0;
    for (int col = 1; col <= COL_DIMENSION; col++)
    {
      if (m_playingBoard[number(row, col)].getOwner() == player)
      {
        count++;
      }
      else
      {
        break;
      }
      if (count == COL_DIMENSION)
      {
        return true;
      }
    }
  }
  for (int col = 1; col <= COL_DIMENSION; col++)
  {
    int count = 0;
    for (int row = 1; row <= ROW_DIMENSION; row++)
    {
      if (m_playingBoard[number(row, col)].getOwner() == player)
      {
        count++;
      }
      else
      {
        break;
      }
      if (count == ROW_DIMENSION)
      {
        return true;
      }
    }
  }

  // diagonal top left to bottom right, assume square
  int diagonal = 0;
  for (int row = 1; row <= ROW_DIMENSION; row++)
  {
    if (m_playingBoard[number(row, row)].getOwner() == player)
    {
      diagonal++;
    }
  }
  if (diagonal == ROW_DIMENSION)
  {
    return true;
  }

  // diagonal bottom left to top right, assume square
  diagonal = 0;
  for (int row = 1; row <= ROW_DIMENSION; row++)
  {
    if (m_playingBoard[number(row, ROW_DIMENSION - row + 1)].getOwner() == player)
    {
      diagonal++;
    }
  }
  if (diagonal == ROW_DIMENSION)
  {
    return true;
  }
  return false;
}

bool Board::isDraw() const
{
  if (validMoves().size() == 0)
  {
    return true;
  }
  return false;
}

bool Board::gameEnded(bool verbose) const
{
  if (isWon(PlayerEnum::O))
  {
    if (verbose)
    {
      std::cout << "Won by O!\n";
    }
    return true;
  }
  if (isWon(PlayerEnum::X))
  {
    if (verbose)
    {
      std::cout << "Won by X!\n";
    }
    return true;
  }
  if (isDraw())
  {
    if (verbose)
    {
      std::cout << "Draw\n";
    }
    return true;
  }
  return false;
}

int Board::hash(PlayerEnum::Player forPlayer) const
{
  int result = 0;
  int multiplier = 1;

  for (int row = 1; row <= ROW_DIMENSION; row++)
  {
    for (int col = 1; col <= COL_DIMENSION; col++)
    {
      if (m_playingBoard[number(row, col)].getOwner() == PlayerEnum::X)
      {
        result += multiplier;
      }
      else if (m_playingBoard[number(row, col)].getOwner() == PlayerEnum::O)
      {
        result += (multiplier * 2);
      }
      multiplier *= 4;
    }
  }
  if (forPlayer == PlayerEnum::O)
  {
    result += multiplier;
  }
  return result;
}

std::string Board::toString() const
{
  std::string lines1(COL_DIMENSION * 8, '=');
  std::string lines2(COL_DIMENSION * 8, '-');
  std::string str = lines1 + "\n\n";
  for (int row = 1; row <= ROW_DIMENSION; row++)
  {
    Field f = m_playingBoard[number(row, 1)];
    std::string rowStr = f.toString();
    for (int col = 2; col <= ROW_DIMENSION; col++)
    {
      Field f = m_playingBoard[number(row, col)];
      rowStr = rowStr + " | " + f.toString();
    }
    str += (rowStr + "\n");
    if (row == ROW_DIMENSION)
    {
      str += "\n";
    }
    else
    {
      str += (lines2 + "\n");
    }
  }
  str += (lines1 + "\n");
  return str;
}

void Board::printToScreen() const
{
  std::cout << toString();
}

Field Board::getField(int number) const
{
  return m_playingBoard[number];
}

void Board::setField(int number, const Field& field)
{
  m_playingBoard[number] = field;
}

int Board::number(int x, int y)
{
  return ((x - 1) * COL_DIMENSION) + y;
}

double Board::getReward() const
{
  if (isWon(PlayerEnum::X)) return 1.0;
  if (isWon(PlayerEnum::O)) return -1.0;
  if (isDraw()) return 0.5;
  return 0.0;
}

//*****************************************************************************
// PlayerToggle

PlayerToggle::PlayerToggle()
{
  m_gammaPlayer = PlayerEnum::X;
  m_opponentPlayer = PlayerEnum::O;
}

PlayerEnum::Player PlayerToggle::getGammaPlayer() const
{
  return m_gammaPlayer;
}

PlayerEnum::Player PlayerToggle::getOpponentPlayer() const
{
  return m_opponentPlayer;
}

void PlayerToggle::toggle()
{
  if (m_gammaPlayer == PlayerEnum::X)
  {
    m_gammaPlayer = PlayerEnum::O;
    m_opponentPlayer = PlayerEnum::X;
  }
  else
  {
    m_gammaPlayer = PlayerEnum::X;
    m_opponentPlayer = PlayerEnum::O;
  }
}

//*****************************************************************************
// random_element

template <typename I>
I random_element(I begin, I end)
{
  const unsigned long n = std::distance(begin, end);
  const unsigned long divisor = RAND_MAX / n;

  unsigned long k;
  do { k = std::rand() / divisor; } while (k >= n);

  std::advance(begin, k);
  return begin;
}

//*****************************************************************************
// Engine

void Engine::resetBetweenGames()
{
  if (verbose())
  {
    std::cout << "Engine::resetBetweenGames()" << std::endl;
  }
}

//*****************************************************************************
// RandomEngine

int RandomEngine::makeMove(const Board& board, PlayerEnum::Player forPlayer, bool trainingEnabled)
{
  std::vector<int> validMoves = board.validMoves();
  return *random_element(validMoves.begin(), validMoves.end());
}

//*****************************************************************************
// MinMaxEngine - Use minmax search

int MinMaxEngine::makeMove(const Board& board, PlayerEnum::Player forPlayer, bool trainingEnabled)
{
  m_player = forPlayer;
  std::pair<int, int> res = alpha_beta(forPlayer, board, -1000, +1000);
  if (res.second == 0)
  {
    std::vector<int> validMoves = board.validMoves();
    return *random_element(validMoves.begin(), validMoves.end());
  }
  else
  {
    return res.second;
  }
}

std::pair<int, int> MinMaxEngine::alpha_beta(PlayerEnum::Player player, const Board& board, int alpha, int beta)
{
  int move = 0;
  std::vector<int> validMoves = board.validMoves();

  PlayerEnum::Player otherPlayer = PlayerEnum::getOtherPlayer(player);
  if (player == m_player)
  {
    for (std::vector<int>::const_iterator iter = validMoves.begin(); iter != validMoves.end(); ++iter)
    {
      int m = *iter;
      Board child = board.copy();
      child.move(m, player);
      if (child.isWon(player))
      {
        return std::make_pair(1, m);
      }
      if (child.isDraw())
      {
        return std::make_pair(0, m);
      }
      if (child.isWon(otherPlayer))
      {
        return std::make_pair(-1, m);
      }
      std::pair<int, int> score = alpha_beta(otherPlayer, child, alpha, beta);
      if (score.first > alpha)
      {
        alpha = score.first;
        move = m;
      }
      if (alpha >= beta)
      {
        return std::make_pair(alpha, m);
      }
    }
    return std::make_pair(alpha, move);
  }
  else
  {
    for (std::vector<int>::const_iterator iter = validMoves.begin(); iter != validMoves.end(); ++iter)
    {
      int m = *iter;
      Board child = board.copy();
      child.move(m, player);
      if (child.isWon(player))
      {
        return std::make_pair(-1, m);
      }
      if (child.isDraw())
      {
        return std::make_pair(0, m);
      }
      if (child.isWon(otherPlayer))
      {
        return std::make_pair(1, m);
      }
      std::pair<int, int> score = alpha_beta(otherPlayer, child, alpha, beta);
      if (score.first < beta)
      {
        beta = score.first;
        move = m;
      }
      if (alpha >= beta)
      {
        return std::make_pair(beta, m);
      }
    }
    return std::make_pair(beta, move);
  }
}

//*****************************************************************************
// RLEngine - reinforcement learning engine

QMatrix RLEngine::m_QMatrix;

RLEngine::RLEngine(double epsilon)
{
  initialize(epsilon);
}

void RLEngine::initialize(double epsilon)
{
  m_epsilon = epsilon;
  m_alpha = 0.3;
  m_gamma = 0.9;
  m_verbose = false;
}

bool RLEngine::verbose() const
{
  return m_verbose;
}

void RLEngine::verbose(bool v)
{
  m_verbose = v;
}

int RLEngine::makeMove(const Board& board, PlayerEnum::Player forPlayer, bool trainingEnabled)
{
  int move(0);
  if (m_verbose)
  {
    std::cout << "makeMove: START: forPlayer = " << PlayerEnum::toString(forPlayer) << std::endl;
  }
  if (trainingEnabled && (static_cast <double>(rand()) / static_cast <double>(RAND_MAX)) < m_epsilon)
  {
    std::vector<int> validMoves = board.validMoves();
    move = *random_element(validMoves.begin(), validMoves.end());
    if (m_verbose)
    {
      std::cout << "makeMove: random move " << move << std::endl;
    }
  }
  else
  {
    if (m_verbose)
    {
      std::cout << "makeMove: len(QMatrix) = " << m_QMatrix.size() << std::endl;
    }
    int state_key = camak(board, forPlayer, m_verbose);
    if (m_verbose)
    {
      std::cout << "makeMove: state_key = " << state_key << std::endl;
    }
    // call to camak above should ensure this always exists
    QEntry Qe = m_QMatrix[state_key];
    if (m_verbose)
    {
      std::cout << "makeMove: Qe = " << std::endl;
      for (QEntry::const_iterator iter = Qe.begin(); iter != Qe.end(); ++iter)
      {
          std::cout << iter->first << " => " << iter->second << std::endl;
      }
    }
    if (forPlayer == PlayerEnum::X)
    {
      move = stochastic_argmax(Qe);
    }
    else if (forPlayer == PlayerEnum::O)
    {
      move = stochastic_argmin(Qe);
    }
    if (m_verbose)
    {
      std::cout << "makeMove: move = " << move << std::endl;
    }
  }

  if (trainingEnabled)
  {
    q_learn(board.copy(), move, forPlayer);
  }
  // board.move(move, forPlayer);
  if (m_verbose)
  {
    std::cout << "makeMove: END: move = " << move << std::endl;
  }
  if (move == 0)
  {
    std::cout << "makeMove: move = " << move << std::endl;
  }
  return move;
}

void RLEngine::q_learn(const Board& board, int move, PlayerEnum::Player forPlayer) const
{
  if (m_verbose)
  {
    std::cout << "q_learn: move = " << move << ", forPlayer = " << PlayerEnum::toString(forPlayer) << std::endl;
  }
  int state_key = camak(board, forPlayer, m_verbose);
  Board next_board = board.copy();
  next_board.move(move, forPlayer);
  double reward = next_board.getReward();
  if (m_verbose)
  {
    std::cout << "q_learn: reward = " << reward << std::endl;
  }
  int next_state_key = camak(next_board, PlayerEnum::getOtherPlayer(forPlayer), m_verbose);

  double expected(0.0);
  if (next_board.gameEnded())
  {
    expected = reward;
  }
  else
  {
    QEntry next_Qe = m_QMatrix[next_state_key];
    if (forPlayer == PlayerEnum::X)
    {
      double min = std::numeric_limits<double>::max();
      for (QEntry::const_iterator iter = next_Qe.begin(); iter != next_Qe.end(); ++iter)
      {
        if (iter->second < min)
        {
          min = iter->second;
        }
      }
      if (m_verbose)
      {
          std::cout << "q_learn: min = " << min << std::endl;
      }
      expected = reward + (m_gamma * min);
    }
    else if (forPlayer == PlayerEnum::O)
    {
      double max(-std::numeric_limits<double>::max());
      for (QEntry::const_iterator iter = next_Qe.begin(); iter != next_Qe.end(); ++iter)
      {
        if (iter->second > max)
        {
          max = iter->second;
        }
      }
      if (m_verbose)
      {
          std::cout << "q_learn: max = " << max << std::endl;
      }
      expected = reward + (m_gamma * max);
    }
  }
  if (m_verbose)
  {
    std::cout << "q_learn: expected = " << expected << std::endl;
  }
  double change = m_alpha * (expected - m_QMatrix[state_key][move]);
  m_QMatrix[state_key][move] += change;
}

int RLEngine::stochastic_argmax(const QEntry& Qe)
{
  int move(0);
  std::vector<int> maxQ;
  double max = -std::numeric_limits<double>::max();
  for (QEntry::const_iterator iter = Qe.begin(); iter != Qe.end(); ++iter)
  {
    if (iter->second > max)
    {
      maxQ.clear();
      maxQ.push_back(iter->first);
      max = iter->second;
      // std::cout << "new max: " << iter->first << " => " << iter->second << std::endl;
    }
    else if (iter->second == max)
    {
      maxQ.push_back(iter->first);
    }
  }
  if (maxQ.size() > 1)
  {
    // best_options = [move for move in list(Qs.keys()) if Qs[move] == min_or_maxQ]
    move = *random_element(maxQ.begin(), maxQ.end());
  }
  else if (maxQ.size() > 0)
  {
    move = maxQ.front();
  }
  if (move == 0)
  {
    std::cout << "argmax: move = 0, len(maxQ) = " << maxQ.size() << ": Qe = \n";
    for (QEntry::const_iterator iter = Qe.begin(); iter != Qe.end(); ++iter)
    {
      std::cout << iter->first << " => " << iter->second << std::endl;
    }
  }
  return move;
}

int RLEngine::stochastic_argmin(const QEntry& Qe)
{
  int move(0);
  std::vector<int> minQ;
  double min = std::numeric_limits<double>::max();
  for (QEntry::const_iterator iter = Qe.begin(); iter != Qe.end(); ++iter)
  {
    if (iter->second < min)
    {
      minQ.clear();
      minQ.push_back(iter->first);
      min = iter->second;
      // std::cout << "new min: " << iter->first << " => " << iter->second << std::endl;
    }
    else if (iter->second == min)
    {
      minQ.push_back(iter->first);
    }
  }
  if (minQ.size() > 1)
  {
    // best_options = [move for move in list(Qs.keys()) if Qs[move] == min_or_maxQ]
    move = *random_element(minQ.begin(), minQ.end());
  }
  else if (minQ.size() > 0)
  {
    move = minQ.front();
  }
  if (move == 0)
  {
    std::cout << "argmin: move = 0, Qe\n";
    for (QEntry::const_iterator iter = Qe.begin(); iter != Qe.end(); ++iter)
    {
      std::cout << iter->first << " => " << iter->second << std::endl;
    }
  }
  return move;
}

// create and maybe add key
int RLEngine::camak(const Board& board, PlayerEnum::Player forPlayer, bool verbose)
{
  double default_Qvalue = 1.0;
  if (verbose)
  {
    std::cout << "camak: START" << std::endl;
    std::cout << "camak: board ..." << std::endl;
    board.printToScreen();
  }
  int state_key = board.hash(forPlayer);
  if (verbose)
  {
    std::cout << "camak: forPlayer = " << pe(forPlayer) << ", state_key = " << state_key << std::endl;
  }
  // static QMatrix
  if (m_QMatrix.find(state_key) == m_QMatrix.end())
  {
    std::vector<int> moves = board.validMoves();
    QEntry defmoves;
    for (std::vector<int>::const_iterator iter = moves.begin(); iter != moves.end(); ++iter)
    {
      defmoves[*iter] = default_Qvalue;
    }
    m_QMatrix[state_key] = defmoves; //  {moves: default_Qvalue for moves in moves}
    if (verbose)
    {
      std::cout << "camak: added key = " << state_key << std::endl;
      std::cout << "camak: len(Qm) = " << m_QMatrix.size() << std::endl;
    }
  }
  if (verbose)
  {
    std::cout << "camak: END" << std::endl;
  }
  return state_key;
}

//*****************************************************************************
// TimeSeries

TimeSeries::TimeSeries()
{
}

void TimeSeries::add(int numPlayer1Wins, int numPlayer2Wins, int numDraws)
{
  m_player1Wins.push_back(numPlayer1Wins);
  m_player2Wins.push_back(numPlayer2Wins);
  m_draws.push_back(numDraws);
}

void TimeSeries::write(const std::string& filename) const
{
  std::ofstream ofs(filename, std::ofstream::out);

  if (!ofs.good())
  {
    std::cerr << "Unable to open file " << filename << std::endl;
    return;
  }
  int i = 1;
  ofs << "Games:,";
  for (std::vector<int>::const_iterator iter = m_player1Wins.begin(); iter != m_player1Wins.end(); ++iter)
  {
    ofs << "M" << i << ",";
    i++;
  }
  ofs << std::endl;
  ofs << "Player1 wins:,";
  for (std::vector<int>::const_iterator iter = m_player1Wins.begin(); iter != m_player1Wins.end(); ++iter)
  {
    ofs << *iter << ",";
  }
  ofs << std::endl;
  ofs << "Player2 wins:,";
  for (std::vector<int>::const_iterator iter = m_player2Wins.begin(); iter != m_player2Wins.end(); ++iter)
  {
    ofs << *iter << ",";
  }
  ofs << std::endl;
  ofs << "Draws:,";
  for (std::vector<int>::const_iterator iter = m_draws.begin(); iter != m_draws.end(); ++iter)
  {
    ofs << *iter << ",";
  }
  ofs << std::endl;
}

//*****************************************************************************
// AutoPlay

GameResultEnum::Result AutoPlay::play(Engine& engine, Engine& opponent, PlayerEnum::Player enginePlayer, PlayerEnum::Player opponentPlayer, bool trainingEnabled)
{
  engine.resetBetweenGames();
  opponent.resetBetweenGames();

  Board board;
  GameResultEnum::Result gameResult = GameResultEnum::NO_RESULT;
  int moveNum = 0;

  while (gameResult == GameResultEnum::NO_RESULT)
  {
    moveNum++;
    if (moveNum > 1 || enginePlayer == PlayerEnum::O)
    {
      //std::cout << "engine.makeMove, enginePlayer = " << pe(enginePlayer) << std::endl;
      int engineMove = engine.makeMove(board.copy(), enginePlayer, trainingEnabled);
      board.move(engineMove, enginePlayer);
      if (board.gameEnded())
      {
        if (board.isWon(enginePlayer))
        {
          gameResult = GameResultEnum::ENGINE_WON;
        }
        else
        {
          gameResult = GameResultEnum::DRAW;
        }
      }
    }
    if (gameResult == GameResultEnum::NO_RESULT)
    {
      //std::cout << "opponent.makeMove, enginePlayer = " << pe(opponentPlayer) << std::endl;
      int opponentMove = opponent.makeMove(board.copy(), opponentPlayer, trainingEnabled);
      board.move(opponentMove, opponentPlayer);
      if (board.gameEnded())
      {
        if (board.isWon(opponentPlayer))
        {
          gameResult = GameResultEnum::OPPONENT_WON;
        }
        else
        {
          gameResult = GameResultEnum::DRAW;
        }
      }
    }
  }
  return gameResult;
}

//*****************************************************************************
// Training

Training::Training()
  : m_autoplay()
  , m_runs(10000)
{
}

void Training::train(Engine& engine)
{
  RLEngine opponentEngine(0.9);
  PlayerToggle playerToggle;

  //opponentEngine.verbose(true);
  //engine.verbose(true);

  for (int i = 0; i < m_runs; i++)
  {
    m_autoplay.play(engine, opponentEngine, playerToggle.getGammaPlayer(), playerToggle.getOpponentPlayer(), true);
    playerToggle.toggle();
  }
}

//*****************************************************************************
// HumanPlay

void HumanPlay::play(int sockfd)
{
  std::shared_ptr<Engine> gammaEngine;

  // gammaEngine = std::make_shared<RandomEngine>();
  gammaEngine = std::make_shared<RLEngine>();

  Board board;

  int inputToken = -1;
  while (inputToken != 0)
  {
    system("clear");
    printf("欢迎来到TIC游戏室，请开始与AI对战:\n");
    board.printToScreen();
    char buf[MAXLINE];
    std::cout << "Your move: ";
    if (fgets(buf, MAXLINE, stdin) == NULL) {
        break;
    }
    inputToken = buf[0] - '0';

    write(sockfd, buf, strlen(buf));

    if (inputToken == 0) break;

    if (board.isValid(inputToken))
    {
      board.move(inputToken, PlayerEnum::O);
      bool isWon = checkWon(board, PlayerEnum::O, "You Won");
      bool isDraw = checkDraw(board);

      if (!isWon && !isDraw)
      {
        int n = read(sockfd, buf, MAXLINE);
        if (n == 0)
			    printf("the other side has been closed.\n");
        int move = buf[0] - '0';
        // int move = gammaEngine->makeMove(board.copy(), PlayerEnum::X, false);
        board.move(move, PlayerEnum::X);
        bool isWon = checkWon(board, PlayerEnum::X, "Engine Won");
        if (isWon)
        {
          system("clear");
          printf("欢迎来到TIC游戏室，请开始与AI对战:\n");
          printf("Engine Won！\n");
          sleep(1);
          gammaEngine->resetBetweenGames();
        }
        std::cout << std::endl;
      }
      else
      {
        system("clear");
        printf("欢迎来到TIC游戏室，请开始与AI对战:\n");
        printf("Draw！\n");
        sleep(1);
        gammaEngine->resetBetweenGames();
      }
    }
    else
    {
      std::cout << "Invalid Move\n";
      sleep(1);
    }
  }
}

bool HumanPlay::checkWon(Board& board, PlayerEnum::Player owner, const std::string& message) const
{
  if (board.isWon(owner))
  {
    board.printToScreen();
    std::cout << message << std::endl;
    board.initialize();
    return true;
  }
  return false;
}

bool HumanPlay::checkDraw(Board& board) const
{
  if (board.validMoves().size() == 0)
  {
    board.printToScreen();
    std::cout << "Draw...\n";
    board.initialize();
    return true;
  }
  return false;
}

//*****************************************************************************
// SelfPlay

SelfPlay::SelfPlay()
  : m_gammaEngine(std::make_shared<RLEngine>(0.9))
  //, m_opponentEngine(std::make_shared<RLEngine>())
  , m_opponentEngine(std::make_shared<RandomEngine>())
  , m_selfPlayGames(5000)
  , m_selfPlayMatches(10)
{
}

void SelfPlay::play()
{
  TimeSeries overallSeries;
  for (int j = 0; j < m_selfPlayMatches; j++)
  {
    m_training.train(*m_gammaEngine);
    int gammaWins = 0;
    int opponentWins = 0;
    int draws = 0;
    TimeSeries timeSeries;

    PlayerToggle playerToggle;

    //m_gammaEngine->verbose(true);
    //m_opponentEngine->verbose(true);

    for (int i = 0; i < m_selfPlayGames; i++)
    {
      GameResultEnum::Result gameResult = m_autoPlay.play(*m_gammaEngine, *m_opponentEngine, playerToggle.getGammaPlayer(), playerToggle.getOpponentPlayer(), false);
      if (gameResult == GameResultEnum::DRAW) draws++;
      else if (gameResult == GameResultEnum::ENGINE_WON) gammaWins++;
      else if (gameResult == GameResultEnum::OPPONENT_WON) opponentWins++;

      timeSeries.add(gammaWins, opponentWins, draws);
      playerToggle.toggle();
    }

    std::cout << "*** Match Results ***" << std::endl;
    std::cout << "Gamma Wins   : " << gammaWins << std::endl;
    std::cout << "Opponent Wins: " << opponentWins << std::endl;
    std::cout << "Draws:         " << draws << std::endl;

    std::ostringstream gs;
    gs << "series/gamma-series-" << j << ".csv";
    timeSeries.write(gs.str());
    overallSeries.add(gammaWins, opponentWins, draws);
  }

  overallSeries.write("series/overall-series.csv");
}


//*****************************************************************************
// main

int main(int argc, char** argv)
{
  struct sockaddr_in servaddr;
	// char buf[MAXLINE];
	int sockfd;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr.s_addr);
	servaddr.sin_port = htons(SERV_PORT);

	connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  
  // system("clear");
  // printf("欢迎来到TIC游戏室，请开始与AI对战:\n");
  // SelfPlay selfPlay;


  HumanPlay humanPlay;

  // selfPlay.play();
  humanPlay.play(sockfd);
  std::cout << "你已退出游戏" << std::endl;
}