var ServerAddress = "http://192.168.1.100";

var TileColorFirst = cc.color(255, 255, 255);
var DebugTileColorFirst = cc.color(100, 100, 100);
var TileColorSecond = cc.color(0, 0, 0);

var PlayerFirstColor = cc.color(255, 0, 0);
var PlayerSecondColor = cc.color(0, 255, 0);

var ColorTurn = cc.color(255, 255, 255);
var ColorWaiting = cc.color(100, 100, 100);

var DrawDebug = false;

var PlayerID = -1;
var GameID = -1;
var CurrentPlayerID = -1;
var DebugID = -1;
var InverseMoves = false;

var TileSize;
var PawnSize;
var WinSize;
var PawnRows = 3;
var RequestScheduleTime = 1.0;
var GameLayer;

var BoardSize = cc.size(8, 8);

var PawnsArray = new Array();

var UseSprites = true;

var MainScene;

var pawn = function pawn(BoardPosition, Size, DrawColor, IsQueen, PlayerNumber)
{
    this.BoardPosition = BoardPosition;
    this.DrawNode = cc.DrawNode.create();

    var PositionFrom = cc.p((TileSize.width - Size.width)/2, (TileSize.height - Size.height)/2);
    if(UseSprites)
    {
        this.Sprite = cc.Sprite.create(PlayerNumber === 1 ? "pawn1.png" : "pawn2.png");
        this.Sprite.setAnchorPoint(cc.p(0, 0));
        var Position = cc.p(BoardPosition.x * TileSize.width, WinSize.height - (BoardPosition.y + 1) * TileSize.height);
        this.Sprite.setPosition(Position);
        var Scale = TileSize.width / this.Sprite.getContentSize().width;
        this.Sprite.setScale(Scale);
        this.PawnImg = this.Sprite;
    }
    else
    {
        this.DrawNode.drawRect(cc.p(PositionFrom.x, PositionFrom.y),
            cc.p(TileSize.width - PositionFrom.x, TileSize.height - PositionFrom.y), DrawColor);
        var Position = cc.p(BoardPosition.x * TileSize.width, WinSize.height - (BoardPosition.y + 1) * TileSize.height);
        this.DrawNode.setAnchorPoint(cc.p(0.5, 0.5));
        this.DrawNode.setPosition(Position);
        this.PawnImg = this.DrawNode;
    }
    this.IsQueen = IsQueen;
    this.IsAlive = true;
    this.PlayerNumber = PlayerNumber;
};

var AddPawnAtBoardPosition = function(ParentLayer, BoardPosition, PlayerNumber, IsQueen)
{
    var Pawn = new pawn(BoardPosition, PawnSize, PlayerNumber == 0 ? PlayerFirstColor : PlayerSecondColor, IsQueen, PlayerNumber);
    ParentLayer.addChild(Pawn.PawnImg);
    PawnsArray.push(Pawn);
};

var GetPawnAtBoardPosition = function(Position)
{
    for(var IndexCounter = 0;
        IndexCounter < PawnsArray.length;
        IndexCounter++)
    {
        if(cc.pointEqualToPoint(PawnsArray[IndexCounter].BoardPosition, Position))
        {
            if(PawnsArray[IndexCounter].IsAlive)
            {
                return(PawnsArray[IndexCounter]);
            }
        }
    }
    return(0);
};

function SetPawnBoardPosition(Pawn, To)
{
    Pawn.BoardPosition = To;
    var Position = cc.p(To.x * TileSize.width, WinSize.height - (To.y + 1) * TileSize.height);
    Pawn.PawnImg.setPosition(Position);
    Pawn.PawnImg.setLocalZOrder(1);
}

function MovePawnToBoardPosition(From, To)
{
    var Pawn = GetPawnAtBoardPosition(From);
    if(Pawn !== 0)
    {
        SetPawnBoardPosition(Pawn, To);
    }
}

var game_over_layer = cc.LayerColor.extend({
    ctor : function (Color, Width, Height, Won) {
        this._super(Color, Width, Height);
        this.Size = cc.p(Width, Height);
        this.init(Won);
    },
    init : function (Won) {
        var Text;
        if(Won)
        {
            Text = "Congratulations, you win!";
        }
        else
        {
            Text = "You loose."
        }

        var Label = cc.LabelTTF.create(Text, "Arial", 40);
        Label.setPosition(this.Size.x / 2, this.Size.y / 2);
        this.addChild(Label, 1);

        var DrawRect = cc.DrawNode.create();
        var RectSize = cc.p(this.Size.x / 2, this.Size.y / 16);
        DrawRect.drawRect(cc.p(RectSize.x / -2, RectSize.y / -2), cc.p(RectSize.x / 2, RectSize.y / 2), cc.color(255, 255, 255, 100));
        DrawRect.setAnchorPoint(cc.p(0.5, 0.5));
        DrawRect.setPosition(this.Size.x / 2, this.Size.y / 4);
        DrawRect.setTag(123456);
        this.addChild(DrawRect);

        var Again = cc.LabelTTF.create("Try again", "Arial", 20);
        Again.setPosition(this.Size.x / 2, this.Size.y / 4);
        this.addChild(Again, 1);

        var TouchListener = cc.EventListener.create({
            event: cc.EventListener.TOUCH_ONE_BY_ONE,
            swallowTouches: true,
            onTouchBegan: function (touch, event) {
                var Result = false;
                var target = event.getCurrentTarget();
                var TargetTag = event.getCurrentTarget().getTag();
                if(TargetTag === 123456)
                {
                    // NOTE(maciek): Start new game
                    var GameLayer = new game_layer();
                    GameLayer.setTag(321);
                    GameLayer.setPosition(cc.p(0, 0));
                    MainScene.addChild(GameLayer);
                    GameLayer.init();
                    target.getParent().removeFromParent(true);
                }
                return (Result);
            },
            onTouchMoved: function (touch, event) {
            },
            onTouchEnded: function (touch, event) {
            }
        });

        cc.eventManager.addListener(TouchListener, DrawRect);
    }
});

var game_layer = cc.LayerColor.extend({
    ctor : function(color, width, height) {
        this._super(color, width, height);
    },
    init : function() {
        TileSize = cc.size(WinSize.width / BoardSize.width, WinSize.height / BoardSize.height);
        PawnSize = cc.size(TileSize.width / 2, TileSize.height / 2);

        PawnsArray = [];

        for (var Y = 0;
             Y < BoardSize.height;
             Y++)
        {
            for (var X = 0;
                 X < BoardSize.width;
                 X++)
            {
                var IsOdd = (X + Y) % 2;

                if(DrawDebug)
                {   var Label = cc.LabelTTF.create("(" + X + ", " + Y + ")", "Arial", 10);
                    Label.setPosition(cc.p(0, 0));
                    Label.setAnchorPoint(cc.p(0,0));
                    Tile.addChild(Label, 1);
                }

                if(IsOdd)
                {
                    var Tile = cc.DrawNode.create();
                    Tile.drawRect(cc.p(0, 0), cc.p(TileSize.width, TileSize.height),
                        IsOdd ? TileColorSecond : TileColorFirst);
                    Tile.setAnchorPoint(cc.p(0, 0));
                    var Position = cc.p(X * TileSize.width, WinSize.height - (Y + 1) * TileSize.height);
                    Tile.setPosition(Position);
                    this.addChild(Tile, 0);

                    var PlayerNumber = -1;
                    if(Y < PawnRows)
                    {
                        PlayerNumber = 0;
                    }
                    else if(Y > (BoardSize.height - 1 - PawnRows))
                    {
                        PlayerNumber = 1;
                    }

                    if(PlayerNumber !== -1)
                    {
                        AddPawnAtBoardPosition(this, cc.p(X, Y), PlayerNumber, false);
                    }
                }
            }
        }

        SendNewGameRequest();
    }
});

var IsValidBoardPosition = function (Position)
{
    var Result = false;
    if(Position.x >= 0 && Position.x <= BoardSize.width && Position.y >= 0 && Position.y <= BoardSize.height &&
       (Position.x + Position.y)%2 === 1)
    {
        Result = true;
    }
    return(Result);
};

var ConvertLocationToBoardSpace = function(Location)
{
    var Result = 0;
    if(Location.x >= 0 && Location.x <= WinSize.width && Location.y >= 0 && Location.y <= WinSize.height)
    {
        Result = cc.p(Math.floor(Location.x/WinSize.width*BoardSize.width),
                      BoardSize.height - 1 - Math.floor(Location.y/WinSize.height*BoardSize.height));
    }
    return(Result);
};

var SetPawnGlobalPosition = function(Pawn, Position)
{
      var Pos = cc.p(Position.x - PawnSize.width, Position.y - PawnSize.height);
      Pawn.PawnImg.setPosition(Pos);
};

var SendRequest = function(Arguments, Callback)
{
    var XHR = cc.loader.getXMLHttpRequest();

    XHR.open("POST", ServerAddress);
    XHR.setRequestHeader("Content-Type", "text/plain;charset=UTF-8");
    Arguments += "$";
    XHR.send(Arguments);
    cc.log("Sent: " + Arguments);

    WaitingForResponse = true;

    GameLayer.setColor(ColorWaiting);

    XHR.onreadystatechange = function()
    {
        if(XHR.readyState == 4 && (XHR.status >= 200 && XHR.status <=207))
        {
            var Response = XHR.responseText;
            cc.log("Response: " + Response);
            Callback(Response);
        }
    }
};

var WaitingForResponse = false;

var KillPawnAtBoardPosition = function(BoardPosition)
{
    cc.log("Killing pawn at: " + BoardPosition.x + " " + BoardPosition.y);
    var Pawn = GetPawnAtBoardPosition(BoardPosition);
    Pawn.IsAlive = false;
    Pawn.PawnImg.removeFromParent(true);
};

var ConvertLetterPos = function(PosX, PosY)
{
    var PositionX = PosX.charCodeAt(0) - 97;
    var PositionY = PosY.charCodeAt(0) - 48;
    var Result = cc.p(PositionX, PositionY);
    return(Result);
};

var PromoteToQueen = function(Pawn)
{
    if(!Pawn.IsQueen)
    {
        var PositionFrom = cc.p((TileSize.width - PawnSize.width)/4, (TileSize.height - PawnSize.height)/4);
        Pawn.IsQueen = true;

        if(UseSprites)
        {
            var Crown = cc.Sprite.create("crown.png");
            var Scale = Pawn.Sprite.getScale() / 1.5;
            Crown.setScale(Scale);
           // Crown.setAnchorPoint(cc.p(0, 0));
            Crown.setPosition(cc.p(TileSize.width / 1.7, TileSize.height / 1.6));
            Pawn.Sprite.addChild(Crown);
        }
        else
        {
            Pawn.DrawNode.clear();
            var DrawColor;
            if(Pawn.PlayerNumber == 0)
            {
                DrawColor = PlayerFirstColor;
            }
            else
            {
                DrawColor = PlayerSecondColor;
            }

            Pawn.DrawNode.drawRect(cc.p(PositionFrom.x, PositionFrom.y),
                cc.p(TileSize.width - PositionFrom.x, TileSize.height - PositionFrom.y), DrawColor);
        }
    }
};

var InvertPosition = function(Position)
{
    var Result = cc.p(BoardSize.width - 1 - Position.x, BoardSize.height - 1 - Position.y);
    return(Result);
};

var PromoteIfPossible = function(BoardPosition)
{
    var CandidateToPromote = GetPawnAtBoardPosition(BoardPosition);
    if((BoardPosition.y === 0 && CandidateToPromote.PlayerNumber === 1) ||
        (BoardPosition.y === (BoardSize.height - 1) && CandidateToPromote.PlayerNumber === 0))
    {
        PromoteToQueen(CandidateToPromote);
    }
};

var SendRequestToMove = function(From, To, Inverse)
{
    var ToTemp = To;
    if(Inverse)
    {
        From = InvertPosition(From);
        To = InvertPosition(To);
    }

    var FromX = String.fromCharCode(97 + From.x);
    var ToX = String.fromCharCode(97 + To.x);

    var Arguments = "checkerIO&&move&&id=" + CurrentPlayerID + "&&from=" + FromX + From.y + "&&to=" + ToX + To.y;

    var Response = function(Message) {
        WaitingForResponse = false;

        if(Message.includes("CHECKERIO"))
        {
            if(Message.includes("MOVE_OK"))
            {
                var Splitted = Message.split("&&");
                for(var ParamIterator = 0;
                    ParamIterator < Splitted.length;
                    ParamIterator++)
                {
                    var Param = Splitted[ParamIterator].split("=");
                    if (Param.length === 2)
                    {
                        var Name = Param[0];
                        var Value = Param[1];

                        if (Name.includes("current_player_id"))
                        {
                            CurrentPlayerID = parseInt(Value);
                        }
                        else if (Name.includes("killed_pawn"))
                        {
                            var PawnPosition = ConvertLetterPos(Value[0], Value[1]);
                            if(InverseMoves)
                            {
                                PawnPosition = InvertPosition(PawnPosition);
                            }
                            KillPawnAtBoardPosition(PawnPosition);
                        }
                        else if (Name.includes("winner"))
                        {
                            var WinnerID = parseInt(Value);
                            var GameScene = MainScene.getChildByTag(321);
                            GameScene.removeFromParent(true);

                            var GameOverLayer;
                            if(WinnerID === PlayerID)
                            {
                                // TODO(maciek): Display game won message
                                GameOverLayer = new game_over_layer(cc.color(0, 255, 0), WinSize.width, WinSize.height, true);
                            }
                            else
                            {
                                // TODO(maciek): Display game lost message
                                GameOverLayer = new game_over_layer(cc.color(255, 0, 0), WinSize.width, WinSize.height, false);
                            }

                            MainScene.addChild(GameOverLayer);
                        }
                    }
                }

                PromoteIfPossible(ToTemp);
            }
            else if(Message.includes("MOVE_INVALID"))
            {
                // TODO(maciek): Display error to user
                if(InverseMoves)
                {
                    To = InvertPosition(To);
                    From = InvertPosition(From);
                }
                SetPawnBoardPosition(GetPawnAtBoardPosition(To), From);
                GameLayer.setColor(ColorTurn);
            }
            else
            {
                // TODO(maciek): Handle error message
            }

            if(CurrentPlayerID !== PlayerID)
            {
                ScheduleNextPlayerFinishedRequestToAsk();
            }
            else
            {
                GameLayer.setColor(ColorTurn);
            }
        }
    };
    SendRequest(Arguments, Response);
};

var SendNewGameRequest = function()
{
    var Arguments = "checkerIO&&find_new_game";
    var Response = function(Message) {
        WaitingForResponse = false;
        ParseFoundNewGameMessage(Message);
    };
    SendRequest(Arguments, Response);
};

var ParseFoundNewGameMessage = function(Message)
{
    if(Message.includes("CHECKERIO_FOUND_NEW_GAME"))
    {
        var Splitted = Message.split("&&");
        for(var ParamIterator = 0;
            ParamIterator < Splitted.length;
            ParamIterator++)
        {
            var Param = Splitted[ParamIterator].split("=");
            if(Param.length === 2)
            {
                var Name = Param[0];
                var Value = Param[1];

                if(Name.includes("playerID"))
                {
                    PlayerID = parseInt(Value);
                }
                else if(Name.includes("current_player"))
                {
                    CurrentPlayerID = parseInt(Value);
                }
                else if(Name.includes("move_direction"))
                {
                    var MoveDirection = parseInt(Value);
                    InverseMoves = MoveDirection !== 1;
                }
                else if(Name.includes("game_id"))
                {
                    GameID = parseInt(Value);
                }
                else if(Name.includes("dbgID"))
                {
                    DebugID = parseInt(Value);
                }
            }
        }

        if(CurrentPlayerID !== PlayerID)
        {
            ScheduleNextPlayerFinishedRequestToAsk();
        }
        else
        {
            GameLayer.setColor(ColorTurn);
        }
    }
    else if(Message.includes("CHECKERIO_JOINED_QUEUE"))
    {
        if(PlayerID === -1)
        {
            var Splitted = Message.split("&&");
            for(var ParamIterator = 0;
                ParamIterator < Splitted.length;
                ParamIterator++)
            {
                var Param = Splitted[ParamIterator].split("=");
                if(Param.length === 2)
                {
                    var Name = Param[0];
                    var Value = Param[1];

                    if(Name.includes("player_id"))
                    {
                        PlayerID = parseInt(Value);
                        ScheduleNextFoundNewGameRequest();
                    }
                }
            }
        }
    }
    else if (Message.includes("CHECKERIO_GAME_NOT_YET_FOUND"))
    {
        ScheduleNextFoundNewGameRequest();
    }
};

var ScheduleNextPlayerFinishedRequestToAsk = function()
{
    var sequence = cc.Sequence.create(cc.DelayTime.create(RequestScheduleTime),
        cc.CallFunc.create(function() {
            RequestAskIfOtherPlayerFinished()
        }.bind(this)));

    MainScene.runAction(sequence);
};

var RequestAskIfOtherPlayerFinished = function()
{
    var Arguments = "checkerIO&&other_player_finished&&player_id=" + PlayerID + "&&game_id=" + GameID;
    var Response = function(Message) {
        WaitingForResponse = false;

        if(Message.includes("PLAYER_FINISHED"))
        {
            var LastMovePosition = cc.p(-1, -1);
            var Splitted = Message.split("&&");

            var TempFrom = cc.p(-1, -1);
            for(var ParamIterator = 0;
                ParamIterator < Splitted.length;
                ParamIterator++)
            {
                var Param = Splitted[ParamIterator].split("=");
                if(Param.length === 2)
                {
                    var Name = Param[0];
                    var Value = Param[1];

                    if(Name.includes("from"))
                    {
                        TempFrom = ConvertLetterPos(Value[0], Value[1]);
                    }
                    else if(Name.includes("to"))
                    {
                        var To = ConvertLetterPos(Value[0], Value[1]);
                        if(InverseMoves)
                        {
                            TempFrom = InvertPosition(TempFrom);
                            To = InvertPosition(To);
                        }
                        LastMovePosition = To;
                        MovePawnToBoardPosition(TempFrom, To);
                    }
                    else if(Name.includes("killed_pawn"))
                    {
                        var PawnPosition = ConvertLetterPos(Value[0], Value[1]);
                        if(InverseMoves)
                        {
                            PawnPosition = InvertPosition(PawnPosition);
                        }
                        KillPawnAtBoardPosition(PawnPosition);
                    }
                    else if(Name.includes("current_player"))
                    {
                        CurrentPlayerID = parseInt(Value);
                    }
                    else if(Name.includes("winner"))
                    {
                        var WinnerID = parseInt(Value);
                        var GameScene = MainScene.getChildByTag(321);
                        GameScene.removeFromParent(true);

                        var GameOverLayer;
                        if(WinnerID === PlayerID)
                        {
                            // TODO(maciek): Display game won message
                            GameOverLayer = new game_over_layer(cc.color(0, 255, 0), WinSize.width, WinSize.height, true);
                        }
                        else
                        {
                            // TODO(maciek): Display game lost message
                            GameOverLayer = new game_over_layer(cc.color(255, 0, 0), WinSize.width, WinSize.height, false);
                        }

                        MainScene.addChild(GameOverLayer);
                    }
                }
            }

            GameLayer.setColor(ColorTurn);
            PromoteIfPossible(LastMovePosition);
        }
        else
        {
            ScheduleNextPlayerFinishedRequestToAsk();
        }
    };
    SendRequest(Arguments, Response);
};

var ScheduleNextFoundNewGameRequest = function()
{
    var sequence = cc.Sequence.create(cc.DelayTime.create(RequestScheduleTime),
        cc.CallFunc.create(function() {
            RequestAskIfFoundNewGame()
        }.bind(this)));

    MainScene.runAction(sequence);
};

var RequestAskIfFoundNewGame = function()
{
    var Arguments = "checkerIO&&found_new_game&&player_id=" + PlayerID;
    var Response = function(Message) {
        WaitingForResponse = false;
        ParseFoundNewGameMessage(Message);
    };
    SendRequest(Arguments, Response);
};

var TouchedPawn;
var TouchedPawnPreviousZOrder;

cc.game.onStart = function(){
    //load resources
    cc.LoaderScene.preload(["pawn1.png", "pawn2.png", "crown.png"], function () {
        var MyScene = cc.Scene.extend({
            onEnter:function () {
                this._super();

                WinSize = cc.director.getWinSize();
                MainScene = this;

                GameLayer = new game_layer(cc.color(255, 255, 255), WinSize.width, WinSize.height);
                GameLayer.init();
                GameLayer.setTag(321);
                GameLayer.setPosition(cc.p(0, 0));
                this.addChild(GameLayer);

                var TouchListener = cc.EventListener.create({
                    event: cc.EventListener.TOUCH_ONE_BY_ONE,
                    swallowTouches: true,
                    onTouchBegan: function (touch, event) {
                        var Result = false;
                        if (!WaitingForResponse)
                        {
                            var target = event.getCurrentTarget();
                            var LocationInNode = target.convertToNodeSpace(touch.getLocation());
                            var Pawn = GetPawnAtBoardPosition(ConvertLocationToBoardSpace(LocationInNode));

                            var PlayerNumber = 1;
                            if(Pawn !== 0 && Pawn.PlayerNumber === PlayerNumber)// && ID === CurrentPlayerID && ID === PlayerID)
                            {
                                TouchedPawn = Pawn;
                                TouchedPawnPreviousZOrder = Pawn.PawnImg.getLocalZOrder();
                                TouchedPawn.PawnImg.setLocalZOrder(1000);
                                Result = true;
                            }
                        }
                        return (Result);
                    },
                    onTouchMoved: function (touch, event) {
                        if(TouchedPawn !== 0)
                        {
                            SetPawnGlobalPosition(TouchedPawn, touch.getLocation());
                        }
                    },
                    onTouchEnded: function (touch, event) {
                        if(TouchedPawn !== 0)
                        {
                            var BoardPosition = ConvertLocationToBoardSpace(touch.getLocation());
                            var PawnAtBoardPosition = GetPawnAtBoardPosition(BoardPosition);
                            if(BoardPosition !== 0 && IsValidBoardPosition(BoardPosition) && PawnAtBoardPosition === 0)
                            {
                                SendRequestToMove(TouchedPawn.BoardPosition, BoardPosition, InverseMoves);
                                MovePawnToBoardPosition(TouchedPawn.BoardPosition, BoardPosition);
                            }
                            else
                            {
                                SetPawnBoardPosition(TouchedPawn, TouchedPawn.BoardPosition);
                            }
                            TouchedPawn.PawnImg.setLocalZOrder(TouchedPawnPreviousZOrder);
                        }
                        TouchedPawn = 0;
                    }
                });

                cc.eventManager.addListener(TouchListener, this);
            }
        });
        cc.director.runScene(new MyScene());
    }, this);
};