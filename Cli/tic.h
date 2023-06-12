#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

//*****************************************************************************
// PlayerEnum

class PlayerEnum
{
public:
    enum Player { NONE, O, X };
    // PlayerEnum(const std::string& representation);

    static std::string toString(Player player);

    static PlayerEnum::Player getOtherPlayer(Player player);

    //static const std::string NONE = "_";
    //static const std::string O = "O";
    //static const std::string X = "X";

private:
    Player m_player;
};

//*****************************************************************************
// Field

class Field
{
public:
    Field(int number = 0, PlayerEnum::Player owner = PlayerEnum::NONE);

    Field copy() const;

    PlayerEnum::Player getOwner() const;

    void setOwner(PlayerEnum::Player);

    int getNumber() const;

    std::string toString() const;

private:
    int m_number;

    PlayerEnum::Player m_owner;
};

//*****************************************************************************
// Board

class Board
{
public:
    static const int ROW_DIMENSION = 3;
    static const int COL_DIMENSION = 3;

    Board();

    void initialize();

    void move(int number, PlayerEnum::Player owner);

    Board copy() const;

    std::vector<int> validMoves() const;

    bool isValid(int row, int col = -999) const;

    bool isWon(PlayerEnum::Player player) const;

    bool isDraw() const;

    bool gameEnded(bool verbose = false) const;

    int hash(PlayerEnum::Player forPlayer) const;

    double getReward() const;

    std::string toString() const;

    Field getField(int number) const;

    void setField(int number, const Field& field);

    static int number(int x, int y);

    void printToScreen() const;

private:
    std::vector<Field> m_playingBoard;
};

//*****************************************************************************
// PlayerToggle

class PlayerToggle
{
public:
    PlayerToggle();
    ~PlayerToggle() { };

    PlayerEnum::Player getGammaPlayer() const;

    PlayerEnum::Player getOpponentPlayer() const;

    void toggle();

private:

    PlayerEnum::Player m_gammaPlayer;
    PlayerEnum::Player m_opponentPlayer;
};

//*****************************************************************************
// Engine

class Engine
{
public:
    Engine() { };
    virtual ~Engine() { };

    virtual int makeMove(const Board& board, PlayerEnum::Player forPlayer, bool trainingEnabled) = 0;

    virtual void resetBetweenGames();

    virtual bool verbose() const { return false; }

    virtual void verbose(bool verbose) { }
};

//*****************************************************************************
// RandomEngine

class RandomEngine : public Engine
{
public:
    int makeMove(const Board& board, PlayerEnum::Player forPlayer, bool trainingEnabled);
};

//*****************************************************************************
// MinMaxEngine

class MinMaxEngine : public Engine
{
public:
    int makeMove(const Board& board, PlayerEnum::Player forPlayer, bool trainingEnabled);

private:
    std::pair<int, int> alpha_beta(PlayerEnum::Player player, const Board& board, int alpha, int beta);

    PlayerEnum::Player m_player;
};

//*****************************************************************************
// RLEngine

typedef std::map<int, std::map<int, double>> QMatrix;
typedef std::map<int, double> QEntry;

class RLEngine : public Engine
{
public:
    RLEngine(double epsilon = 0.9);
    ~RLEngine() { }

    int makeMove(const Board& board, PlayerEnum::Player forPlayer, bool trainingEnabled);

    void verbose(bool v);

    bool verbose() const;

private:
    void initialize(double epsilon);

    void q_learn(const Board& board, int move, PlayerEnum::Player forPlayer) const;

    static int stochastic_argmin(const QEntry& Qe);

    static int stochastic_argmax(const QEntry& Qe);

    static int camak(const Board& board, PlayerEnum::Player forPlayer, bool verbose = false);

    double m_epsilon;

    double m_alpha;

    double m_gamma;

    bool m_verbose;

    static QMatrix m_QMatrix;
};

//*****************************************************************************
// TimeSeries

class TimeSeries
{
public:
    TimeSeries();
    ~TimeSeries() { };

    void add(int numPlayer1Wins, int numPlayer2Wins, int numDraws);

    void write(const std::string& filename) const;

private:
    std::vector<int> m_player1Wins;
    std::vector<int> m_player2Wins;
    std::vector<int> m_draws;
};

//*****************************************************************************
// GameResultEnum

class GameResultEnum
{
public:
    enum Result { NO_RESULT = 0, ENGINE_WON = 1, OPPONENT_WON = 2, DRAW = 3 };
};

//*****************************************************************************
// AutoPlay

class AutoPlay
{
public:
    AutoPlay() { };
    ~AutoPlay() { };

    GameResultEnum::Result play(Engine& engine, Engine& opponent, PlayerEnum::Player enginePlayer, PlayerEnum::Player opponentPlayer, bool trainingEnabled);
};

//*****************************************************************************
// Training

class Training
{
public:
    Training();
    ~Training() { };

    void train(Engine& engine);

private:
    AutoPlay m_autoplay;
    int m_runs;

};

//*****************************************************************************
// HumanPlay

class HumanPlay
{
public:
    HumanPlay() { };
    ~HumanPlay() { };

    void play(int fd);
    void p2pPlay(int fd);

    bool checkWon(Board& board, PlayerEnum::Player owner, const std::string& message) const;
    bool checkDraw(Board& board) const;
};

//*****************************************************************************
// SelfPlay

class SelfPlay
{
public:
    SelfPlay();
    ~SelfPlay() { }

    void play();

private:
    std::shared_ptr<Engine> m_gammaEngine;
    std::shared_ptr<Engine> m_opponentEngine;
    int m_selfPlayGames;
    int m_selfPlayMatches;
    AutoPlay m_autoPlay;
    Training m_training;
};