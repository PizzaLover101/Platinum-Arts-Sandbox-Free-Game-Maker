#ifndef __RPGGAME__
#define __RPGGAME__

#include "cube.h"

#define DEFAULTMAP "mansion"
#define DEFAULTMODEL "rc"

enum                            // static entity types
{
	NOTUSED = ET_EMPTY,         // entity slot not in use in map
	LIGHT = ET_LIGHT,           // lightsource, attr1 = radius, attr2 = intensity
	MAPMODEL = ET_MAPMODEL,     // attr1 = angle, attr2 = idx
	PLAYERSTART,                // attr1 = angle, attr2 = team
	ENVMAP = ET_ENVMAP,         // attr1 = radius
	PARTICLES = ET_PARTICLES,
	MAPSOUND = ET_SOUND,
	SPOTLIGHT = ET_SPOTLIGHT,
	TELEDEST, //attr1 = yaw, attr2 = from
	JUMPPAD, //attr1 = Z, attr2 = Y, attr3 = X, attr4 = radius
	CHECKPOINT,
	SPAWN,
	LOCATION,
	RESERVED, //backwards compatibility
	BLIP,
	CAMERA,
	PLATFORMROUTE,
	CRITTER,
	ITEM,
	OBSTACLE,
	CONTAINER,
	PLATFORM,
	TRIGGER,
	MAXENTTYPES
};

struct rpgentity : extentity
{
	//extend if needed
};

#define STATUS_INVALID_GENERIC(type) (((type) <= STATUS_SCRIPT && (type) >= STATUS_LIGHT) || (type) >= STATUS_MAX || (type) <= -1)
#define HASINVENTORY(type) ((type) == ENT_CHAR || (type) == ENT_CONTAINER)

enum
{
	STATUS_HEALTH = 0, //attributes
	STATUS_MANA,
	STATUS_MOVE,
	STATUS_CRIT,
	STATUS_HREGEN,
	STATUS_MREGEN,

	///WARNING: it is critical that these are in the same order as the stats in the stats struct
	STATUS_STRENGTH,
	STATUS_ENDURANCE,
	STATUS_AGILITY,
	STATUS_CHARISMA,
	STATUS_WISDOM,
	STATUS_INTELLIGENCE,
	STATUS_LUCK,

	///WARNING: it is critical that these are in the same order as the skills inside the stats struct
	STATUS_ARMOUR,
	STATUS_DIPLOMACY,
	STATUS_MAGIC,
	STATUS_MARKSMAN,
	STATUS_MELEE,
	STATUS_STEALTH,
	STATUS_CRAFT,

	///WARNING: it is critical that these are in the same order as the ATTACK_* enum below
	STATUS_FIRE_T, //these are thresholds
	STATUS_WATER_T,
	STATUS_AIR_T,
	STATUS_EARTH_T,
	STATUS_ARCANE_T,
	STATUS_MIND_T,
	STATUS_HOLY_T,
	STATUS_DARKNESS_T,
	STATUS_SLASH_T,
	STATUS_BLUNT_T,
	STATUS_PIERCE_T,

	STATUS_FIRE_R, //these are resistances
	STATUS_WATER_R,
	STATUS_AIR_R,
	STATUS_EARTH_R,
	STATUS_ARCANE_R,
	STATUS_MIND_R,
	STATUS_HOLY_R,
	STATUS_DARKNESS_R,
	STATUS_SLASH_R,
	STATUS_BLUNT_R,
	STATUS_PIERCE_R,

	STATUS_STUN,
	STATUS_SILENCE,
	STATUS_DOOM,

	STATUS_LOCK, //objects
	STATUS_MAGELOCK,

	STATUS_DISPEL, //negative strength dispells positive magics, positive strength bad magics
	STATUS_REFLECT, //reflects spells in the direction of the caster
	STATUS_INVIS,
	STATUS_LIGHT,
	STATUS_POLYMORPH,
	STATUS_SIGNAL,
	STATUS_SCRIPT,
	STATUS_MAX
};

enum
{
	ATTACK_NONE = -1,
	ATTACK_FIRE,
	ATTACK_WATER,
	ATTACK_AIR,
	ATTACK_EARTH,
	ATTACK_ARCANE,
	ATTACK_MIND,
	ATTACK_HOLY,
	ATTACK_DARKNESS,
	ATTACK_SLASH,
	ATTACK_BLUNT,
	ATTACK_PIERCE,
	ATTACK_MAX
};

struct journal
{
	const char *name;
	vector<const char *> entries;

	journal() : name(NULL) {}
	~journal() { delete[] name; entries.deletearrays(); }
};

static inline bool htcmp(const char *key, const journal &ref) { return !strcmp(key, ref.name); }

struct response
{
	const char *talk; //text the player can respond with
	const char *dest; //target node
	uint *script; //script to execute when option is selected (optional)

	response(const char *t, const char *d, const char *s) : talk(newstring(t)), dest(newstring(d)), script(s ? compilecode(s) : NULL) {}
	~response()
	{
		delete[] talk;
		delete[] dest;
		freecode(script);
	}
};

struct dialogue
{
	const char *node;
	uint *text;
	uint *script;

	vector<response *> choices; //and consequences?

	const char *str;

	void open()
	{
		execute(script);
		str = executestr(text);
	}
	void close()
	{
		choices.deletecontents();
		DELETEA(str);
	}

	dialogue() : node(NULL), text(NULL), script(NULL), str(NULL) {}
	~dialogue()
	{
		delete[] node;
		delete[] str;
		freecode(text);
		freecode(script);
		choices.deletecontents();
	}
};

struct signal
{
	const char *name;
	vector <uint *> code;

	inline void setcode(uint *body)
	{
		while(code.length()) freecode(code.pop());
		keepcode(code.add(body));
	}

	inline void appendcode(uint *body)
	{
		keepcode(code.add(body));
	}

	signal() : name(NULL) {}
	~signal()
	{
		delete[] name;
		loopv(code) freecode(code[i]);
	}
};

struct script
{
	hashtable<const char *, signal> listeners;
	hashtable<const char *, dialogue> chat;
	dialogue *curnode;

	script() : listeners(hashtable<const char *, signal>(32)),
		chat(hashtable<const char *, dialogue>(32)), curnode(NULL) {}
	~script() {}
};

struct mapscript
{
	hashtable<const char *, signal> listeners;

	mapscript() : listeners(hashtable<const char *, signal>(32)) {}
	~mapscript() {}
};

struct rpgent;
struct rpgchar;
struct rpgitem;
struct use_weapon;

enum
{
	FX_NONE = 0,
	FX_DYNLIGHT = 1 << 0,
	FX_FLARE = 1 << 1, //LENSFLARE
	FX_FIXEDFLARE = 1 << 2,
	FX_MAX = (1 << 3) - 1
};

struct effect
{
	int flags, decal;
	const char *mdl;
	vec spin;

	int particle, colour, fade, gravity;
	float size;

	int lightflags, lightfade, lightradius, lightinitradius;
	vec lightcol, lightinitcol;

	effect() : flags(FX_DYNLIGHT|FX_FIXEDFLARE), decal(DECAL_BURN), mdl(NULL), spin(vec(0, 0, 0)), particle(PART_FIREBALL1), colour(0xFFBF00), fade(500), gravity(50), size(4), lightflags(DL_EXPAND), lightfade(500), lightradius(64), lightinitradius(lightradius), lightcol(vec(1, .9, 0)), lightinitcol(lightcol) {}
	~effect() { delete[] mdl; }

	enum
	{
		PROJ,
		TRAIL, // a trail
		TRAIL_SINGLE, //a trail which only emits once
		DEATH, // death cariant which only emits once
		DEATH_PROLONG // death variant which is intended for a delay that takes a while
	};

	void drawsphere(vec &o, float radius, float size = 1, int type = PROJ, int elapse = 17);
	void drawsplash(vec &o, vec dir, float radius, float size = 1, int type = PROJ, int elapse = 17);
	bool drawline(vec &from, vec &to, float size = 1, int type = PROJ, int elapse = 17);
	void drawwield(vec &from, vec &to, float size = 1, int type = PROJ, int elapse = 17);
	void drawaura(rpgent *d, float size = 1, int type = PROJ, int elapse = 17);
	void drawcircle(vec &o, vec dir, vec &axis, int angle, float radius, float size = 1, int type = PROJ, int elapse = 17);
	void drawcircle(rpgent *d, use_weapon *wep, float size = 1, int type = PROJ, int elapse = 17);
	void drawcone(vec &o, vec dir, vec &axis, int angle, float radius, float size, int type = PROJ, int elapse = 17);
	void drawcone(rpgent *d, use_weapon *wep, float size, int type = PROJ, int elapse = 17);
};

enum
{
	P_TIME =	1 << 0,		// dissapears after set time
	P_DIST =	1 << 1,		// dissapears after set distance
	P_RICOCHET =	1 << 2,		// bounces off walls
	P_VOLATILE =	1 << 3,		// takes effect at current position at end of life (mix with P_TIME or P_DIST)
	P_PROXIMITY =	1 << 4,		// takes effect at current position if there's something in range of the effect (provided they aren't coming towards the projectile)
	P_STATIONARY =	1 << 5,		// does not move (ie mines) - requires proximity or time
	P_PERSIST =	1 << 6,		// persists until expiration; allwos you to hit many things
	P_MAX =		(1 << 7) - 1
};

enum
{
	T_SINGLE = 0,	//hits one
	T_MULTI,	//hits many
	T_AREA,		//persistant effect on area
	T_SELF,		//on self
	T_SMULTI,	//for multi effects on self
	T_SAREA,	//for area effects on self
	T_HORIZ,	//horizontal arc
	T_VERT,		//vertical arc
	T_CONE,		//hits everything inside a cone
	T_MAX
};

struct item;

struct equipment
{
	item *it;
	int use;
	equipment(item *i, int u = 0) : it(i), use(u) {}
};

struct projectile
{
	/**
		<NOTES>
		1) speed is always 100, this can be modulated by changing the magnitude of dir as neccesary
	*/
	rpgchar *owner;
	equipment item, ammo;

	vec o, dir, emitpos;
	int lastemit, gravity;
	bool deleted;
	vector<rpgent *> hits; //used for P_PERSIST - only hits an ent once
	entitylight light;

	int pflags, time, dist;
	int projfx, trailfx, deathfx;
	int radius;
	float elasticity;

	float charge;
	int chargeflags;

	void init(rpgchar *d, equipment *w, equipment *a, int fuzzy, float mult = 1.0f, float speed = 1.0f);
	bool update();
	float travel(float &distance, vector<rpgent *> &victims);
	void render(bool mainpass);
	void drawdeath();
	void dynlight();

	projectile() : owner(NULL), item(NULL), ammo(NULL), o(vec(0, 0, 0)), dir(vec(0, 0, 0)), emitpos(vec(0, 0, 0)), lastemit(lastmillis), gravity(0), deleted(false), pflags(0), time(10000), dist(25000), projfx(0), trailfx(0), deathfx(0), radius(32), elasticity(.8), charge (1.0f), chargeflags(0) {}
	~projectile() {};
};

struct status
{
	int type, duration, remain, strength;
	float variance;

	virtual int value()=0;
	virtual status *dup()=0; //duplicate
	virtual void update(rpgent *victim, rpgent *owner, int resist, int thresh, float mul = 1, float extra = 0)=0;

	status() : type(STATUS_HEALTH), duration(0), remain(0), strength(0), variance(0.0f) {}
	status(int t, int d, int r, int s, float v) : type(t), duration(d), remain(r), strength(s), variance(v) {}
	virtual ~status() {}
};

struct status_generic : status
{
	int value() { return abs(strength); }
	status *dup() { return new status_generic(*this); }
	void update(rpgent *victim, rpgent *owner, int resist, int thresh, float mul = 1, float extra = 0);

	status_generic() {}
	status_generic(int t, int d, int r, int s, float v) : status(t, d, r, s, v) {}
	~status_generic() {}
};

struct status_polymorph : status
{
	const char *mdl;

	int value() {return strength;}
	status *dup()
	{
		status_polymorph *p = new status_polymorph(*this);
		p->mdl = mdl ? newstring(mdl) : NULL;
		return p;
	}
	void update(rpgent *victim, rpgent *owner, int resist, int thresh, float mul, float extra);

	status_polymorph() : mdl(NULL) {}
	~status_polymorph() { delete[] mdl; }
};

struct status_light : status
{
	vec colour;

	int value() { return strength * colour.magnitude(); }
	status *dup() { return new status_light(*this); }
	void update(rpgent *victim, rpgent *owner, int resist, int thresh, float mul, float extra);

	status_light() : colour(vec(0, 0, 0)) {}
	~status_light() {}
};

struct status_signal : status
{
	const char *signal;

	int value() {return strength;}
	status *dup()
	{
		status_signal *n = new status_signal(*this);
		if(signal) n->signal = newstring(signal);
		return n;
	}
	void update(rpgent *victim, rpgent *owner, int resist, int thresh, float mul, float extra);

	status_signal() : signal(NULL) {}
	~status_signal() {delete[] signal;}
};

struct status_script : status
{
	const char *script;
	uint *code;

	int value() {return strength;}
	status *dup()
	{
		status_script *n = new status_script(*this);
		if(script) n->script = newstring(script);
		keepcode(n->code);
		return n;
	}
	void update(rpgent *victim, rpgent *owner, int resist, int thresh, float mul, float extra);

	status_script() : script(NULL), code(NULL) {}
	~status_script() {delete[] script; freecode(code);}
};

struct statusgroup
{
	vector<status *> effects;
	bool friendly;

	const char *icon, *name, *description;

	statusgroup() : friendly(false), icon(NULL), name(NULL), description(NULL) {}
	~statusgroup()
	{
		delete[] icon;
		delete[] name;
		delete[] description;
		effects.deletecontents();
	}

	int strength()
	{
		int res = 0;
		loopv(effects)
			res += effects[i]->value();
		return res;
	}
};

struct areaeffect
{
	rpgent *owner;
	vec o;
	int lastemit, group, elem, radius, fx;

	vector<status *> effects;

	bool update();
	void render();
	void dynlight();

	areaeffect() : owner(NULL), o(vec(0, 0, 0)), lastemit(lastmillis), group(0), elem(ATTACK_NONE), radius(0), fx(0) {}
	~areaeffect() { effects.deletecontents(); }
};

struct inflict;
struct victimeffect
{
	rpgent *owner;
	int group;
	int elem;
	vector<status *> effects;

	bool update(rpgent *victim);
	victimeffect(rpgent *o, inflict *inf, int chargeflags, float mul);
	victimeffect() : owner(NULL), group(0), elem(ATTACK_NONE) {}
	~victimeffect() { effects.deletecontents(); }
};

enum
{
	ENT_INVALID = 0,
	ENT_CHAR,
	ENT_ITEM,
	ENT_OBSTACLE,
	ENT_CONTAINER,
	ENT_PLATFORM,
	ENT_TRIGGER
};

struct rpgent : dynent
{
	int lasttouch; //used for entity touches - mainly to avoid getting overpowered

	struct
	{
		vec4 light; //w == radius x y z == R G B
		float alpha;
		const char *mdl;
	} temp;

	///everything can suffer some status effects, whether this is just invisibility or something more sinister is up for debate
	vector<victimeffect *> seffects;

	///global
	virtual void update()=0;
	virtual void resetmdl()=0;
	virtual void render(bool mainpass)=0;
	virtual int getscript()=0;
	virtual vec blipcol() { return vec(1, 1, 1);}
	virtual const char *getname() const =0;
	virtual const int type()=0;
	virtual void init(int base)=0;
	virtual void getsignal(const char *sig, bool prop = true, rpgent *sender = NULL);

	///character/AI
	virtual void givexp(int xp) {}
	virtual void equip(item *it, int u = 0) {}
	virtual bool dequip(int base, int slots = 0) {return false;}
	virtual void die(rpgent *killer = NULL) {}
	virtual void revive(bool spawn = true) {}
	virtual void hit(rpgent *attacker, use_weapon *weapon, use_weapon *ammo, float mul, int flags, vec dir) = 0;

	///inventory
	virtual item *additem(int base, int q) {return NULL;}
	virtual item *additem(item *it) {return NULL;}
	virtual int drop(item *it, int q, bool spawn) {return 0;}
	virtual int drop(int base, int q, bool spawn = false) {return 0;}
	virtual int pickup(rpgitem *it) {return 0;}
	virtual int getitemcount(int base) {return 0;}
	virtual int getcount(item *it) {return 0;}
	virtual float getweight() {return 0;}

	///{de,con}structors
	rpgent() : lasttouch(0)
	{
		temp.mdl = NULL;
		temp.light = vec4(0, 0, 0, 0);
		temp.alpha = 1;
	}
	virtual ~rpgent()
	{
		seffects.deletecontents();
	}
};

///WARNING keep both of these in sync with the STATUS_* enum above
enum
{
	STAT_STRENGTH = 0,
	STAT_ENDURANCE,
	STAT_AGILITY,
	STAT_CHARISMA,
	STAT_WISDOM,
	STAT_INTELLIGENCE,
	STAT_LUCK,
	STAT_MAX
};

enum
{
	SKILL_ARMOUR = 0,
	SKILL_DIPLOMACY,
	SKILL_MAGIC,
	SKILL_MARKSMAN,
	SKILL_MELEE,
	SKILL_STEALTH,
	SKILL_CRAFT,
	SKILL_MAX
};

struct stats
{
	rpgchar *parent;
	//note delta vars are for spell effects
	///SPECIAL
	int experience, level, statpoints, skillpoints;

	///PRIMARY STATS
	short baseattrs[STAT_MAX];
	short baseskills[SKILL_MAX];

	short deltaattrs[STAT_MAX];
	short deltaskills[SKILL_MAX];

	///DERIVED STATS
	short bonusthresh[ATTACK_MAX];
	short bonusresist[ATTACK_MAX];
	int bonushealth, bonusmana, bonusmovespeed, bonusjumpvel, bonuscarry, bonuscrit;
	float bonushregen, bonusmregen;

	short deltathresh[ATTACK_MAX];
	short deltaresist[ATTACK_MAX];
	int deltahealth, deltamana, deltamovespeed, deltajumpvel, deltacarry, deltacrit;
	float deltahregen, deltamregen;

	///QUERIES
	//no safety checks are done for these 3, make sure to use the ENUM!!
	inline int getattr(int n) const {return max(0, baseattrs[n] + deltaattrs[n]);}
	inline int getskill(int n) const {return max(0, baseskills[n] + deltaskills[n]);}
	inline int critchance() const {return bonuscrit + deltacrit + getattr(STAT_LUCK) / 10;}

	static int neededexp(int level) { return level * (level + 1) * 500; } //standard EXP formula, 0... 1000.... 3000.... 6000.... 10000....
	void givexp(int xp);
	int getmaxhp();
	float gethpregen();
	int getmaxmp();
	float getmpregen();
	int getmaxcarry();
	int getthreshold(int n);
	int getresistance(int n);
	void skillpotency(int n, float &amnt, float &extra);
	void setspeeds(float &maxspeed, float &jumpvel);
	void resetdeltas();

	stats(rpgchar *p) : parent(p), experience(0), level(1), statpoints(0), skillpoints(0)
	{
		loopi(STAT_MAX)
			baseattrs[i] = 30;
		loopi(SKILL_MAX)
			baseskills[i] = 0;
		loopi(ATTACK_MAX)
		{
			bonusthresh[i] = 0;
			bonusresist[i] = 0;
		}
		bonushealth = bonusmana = bonusmovespeed = bonusjumpvel = bonuscarry = bonuscrit = 0;
		bonushregen = bonusmregen = 0;
		resetdeltas();
	}
	~stats() {}
};

struct statreq
{
	short attrs[STAT_MAX];
	short skills[SKILL_MAX];

	statreq()
	{
		loopi(STAT_MAX)
			attrs[i] = 0;
		loopi(SKILL_MAX)
			skills[i] = 0;
	}
	~statreq() {}

	bool compare(statreq &o)
	{
		loopi(STAT_MAX) if(attrs[i] != o.attrs[i])
			return false;
		loopi(SKILL_MAX) if(skills[i] != o.skills[i])
			return false;

		return true;
	}

	bool meet(stats &st)
	{
		loopi(STAT_MAX) if(attrs[i] > st.getattr(i))
			return false;
		loopi(SKILL_MAX) if(skills[i] > st.getskill(i))
			return false;

		return true;
	}
};

enum
{
	USE_CONSUME = 0,
	USE_ARMOUR,
	USE_WEAPON,
	USE_MAX
};

enum
{
	SLOT_LHAND  = 1 << 0,
	SLOT_RHAND  = 1 << 1,
	SLOT_LEGS   = 1 << 2,
	SLOT_ARMS   = 1 << 3,
	SLOT_TORSO  = 1 << 4,
	SLOT_HEAD   = 1 << 5,
	SLOT_FEET   = 1 << 6,
	SLOT_QUIVER = 1 << 7,
	SLOT_MAX    = (1 << 8) - 1,
	SLOT_NUM = 8
};

enum
{
	CHARGE_SPEED = 1 << 0,
	CHARGE_MAG = 1 << 1,
	CHARGE_DURATION = 1 << 2,
	CHARGE_TRAVEL = 1 << 3,
	CHARGE_RADIUS = 1 << 4,
	CHARGE_MAX = (1 << 5) -1
};

struct inflict
{
	int status;
	int element;
	float mul;

	bool compare(inflict *o)
	{
		if(status != o->status || element != o->element || mul != o->mul)
			return false;
		return true;
	}
	inflict(int st, int el, float m) : status(st), element(el), mul(m) {}
	~inflict() {}
};

struct use
{
	const char *name;
	const char *description;
	const char *icon;
	int type; //refers to above
	int script;
	int cooldown;
	int chargeflags;

	vector<inflict *> effects;

	virtual use *dup(use *o = NULL)
	{
		if(!o)
		{
			o = new use(script);
			*o = *this;
		}
		if(name) o->name = newstring(name);
		if(description)  o->description = newstring(description);
		if(icon)  o->icon = newstring(icon);
		loopv(effects)
			o->effects[i] = new inflict(effects[i]->status, effects[i]->element, effects[i]->mul);

		return o;
	}
	virtual void apply(rpgchar *user);
	virtual bool compare(use *o)
	{
		if(type != o->type || script != o->script || cooldown != o->cooldown || chargeflags != o->chargeflags ||
			!name ^ !o->name || (name && strcmp(name, o->name)) ||
			!description ^ !o->description || (description && strcmp(description, o->description)) ||
			!icon ^ !o->icon || (icon && strcmp(icon, o->icon)) ||
			effects.length() != o->effects.length())
			return false;

		loopv(effects) if(!effects[i]->compare(o->effects[i]))
			return false;

		return true;
	}
	use(int s) : name(NULL), description(NULL), icon(NULL), type(USE_CONSUME), script(s), cooldown(500), chargeflags(CHARGE_MAG) {}
	virtual ~use() {delete[] name; delete[] description; delete[] icon; effects.deletecontents();}
};

struct use_armour : use
{
	const char *vwepmdl;
	const char *hudmdl;
	int idlefx;
	statreq reqs;
	int slots;
	int skill;

	use *dup(use *o)
	{
		use_armour *uw = (use_armour *) o;
		if(!uw)
		{
			uw = new use_armour(script);
			*uw = *this;
		}

		if(vwepmdl) uw->vwepmdl = newstring(vwepmdl);
		if(hudmdl) uw->hudmdl = newstring(hudmdl);
// 		conoutf("duplicated %p %p as %p %p", vwepmdl, hudmdl, uw->vwepmdl, uw->hudmdl);

		return use::dup(uw);
	}

	void apply(rpgchar *user);
	bool compare(use *o)
	{
		if(o->type < USE_ARMOUR) return false;
		use_armour *ua = (use_armour *) o;

		if(idlefx != ua->idlefx || slots != ua->slots || skill != ua->skill || !reqs.compare(ua->reqs) ||
			!vwepmdl ^ !ua->vwepmdl || (vwepmdl && strcmp(vwepmdl, ua->vwepmdl)) ||
			!hudmdl ^ !ua->hudmdl || (hudmdl && strcmp(hudmdl, ua->hudmdl)))
			return false;

		return use::compare(o);
	}
	use_armour(int s) : use(s), vwepmdl(NULL), hudmdl(NULL), idlefx(-1), reqs(statreq()), slots(0), skill(SKILL_ARMOUR) {type = USE_ARMOUR;}
	~use_armour() { /*conoutf("use_arm: freeing, %p %p", vwepmdl, hudmdl);*/   delete[] vwepmdl; delete[] hudmdl;}
};

struct use_weapon : use_armour
{
	int range;
	int angle;
	int lifetime;
	int gravity;
	int projeffect, traileffect, deatheffect;
	int cost; //mana/items consumed in use
	int pflags;
	int ammo; //refers to item in ammotype vector<>, -ves use mana
	int target; //self, others, etc
	int radius;
	int kickback, recoil;
	int charge; //how long it takes to charge and the cooldown
	float basecharge; //additional charge, the charge meter only displays the two below -
	float mincharge, maxcharge; // minimum charge to fire and maximum possible charge
	float elasticity, speed;

	use *dup(use *o)
	{
		use_weapon *ua = (use_weapon *) o;
		if(!ua)
		{
			ua = new use_weapon(script);
			*ua = *this;
		}
		return use_armour::dup(ua);
	}
	void apply(rpgchar *user);
	bool compare(use *o)
	{
		if(o->type < USE_WEAPON) return false;
		use_weapon *uw = (use_weapon *) o;

		if(range != uw->range || angle != uw->angle || lifetime != uw->lifetime || gravity != uw->gravity ||
			projeffect != uw->projeffect || traileffect != uw->traileffect || deatheffect != uw->deatheffect ||
			cost != uw->cost || pflags != uw->pflags || ammo != uw->ammo || target != uw->target ||
			radius != uw->radius || kickback != uw->kickback || recoil != uw->recoil || charge != uw->charge ||
			basecharge != uw->basecharge || mincharge != uw->mincharge || maxcharge != uw->maxcharge ||
			elasticity != uw->elasticity || speed != uw->speed)
			return false;

		return use_armour::compare(o);
	}
	use_weapon(int s) : use_armour(s), range(256), angle(60), lifetime(10000), gravity(0), projeffect(0), traileffect(0), deatheffect(0), cost(10), pflags(P_DIST|P_TIME), ammo(-1), target(T_SINGLE), radius(32), kickback(10), recoil(0), charge(0), basecharge(.5f), mincharge(.5f), maxcharge(1.0f), elasticity(0.8), speed(1.0f) {type = USE_WEAPON;}
	~use_weapon() {}
};

struct item
{
	enum
	{
		F_QUEST = 1<<0, //just helps identifying important items - should safe guards be implemented?
		F_CURSED = 1<<1, //item glues itself to the wearer - can be unglued
		F_NATURAL = 1<<2, //like cursed, can be seen if yours, but not looted
		F_STOLEN = 1<<3,
		F_MAX = (1 << 4) -1
	};
	const char *name;
	const char *icon;
	const char *description;
	const char *mdl;

	int quantity;
	int base;

	int script;
	int category;
	int flags;
	int value;
	int maxdurability;
	float weight;
	float durability;
	float recovery;

	vector<use *> uses;

	void getsignal(const char *sig, bool prop = true, rpgent *sender = NULL, int use = -1);

	void init(int base);

	void transfer(item &o)
	{
		delete[] o.name;
		delete[] o.icon;
		delete[] o.description;
		delete[] o.mdl;
		o.uses.deletecontents();

		o = *this;
		if(name) o.name = newstring(name);
		if(icon) o.icon = newstring(icon);
		if(description) o.description = newstring(description);
		if(mdl) o.mdl = newstring(mdl);

		loopv(uses)
			o.uses[i] = uses[i]->dup();
	}

	bool compare(item *o)
	{
		if(base != o->base || durability != o->durability || recovery != o->recovery || script != o->script || category != o->category ||
			flags != o->flags || value != o->value || maxdurability != o->maxdurability || weight != o->weight || uses.length() != o->uses.length())
			return false;

		loopv(uses) if(!uses[i]->compare(o->uses[i]))
			return false;

		if(!name ^ !o->name || (name && strcmp(name, o->name)) || !icon ^ !o->icon || (icon && strcmp(icon, o->icon)) ||
			!description ^ !o->description || (description && strcmp(description, o->description)) ||
			!mdl ^ !o->mdl || (mdl && strcmp(mdl, o->mdl)))
			return false;

		return true;
	}

	item() : name(NULL), icon(NULL), description(NULL), mdl(newstring(DEFAULTMODEL)), quantity(1), base(-1), script(2), category(0), flags(0), value(0), maxdurability(0), weight(0), durability(0), recovery(1) {}
	~item()
	{
		delete[] name;
		delete[] icon;
		delete[] description;
		delete[] mdl;
		uses.deletecontents();
	}
};

struct rpgitem : rpgent, item
{
	int lastupdate;

	void update();
	void resetmdl();
	void render(bool mainpass);
	const char *getname() const;
	vec blipcol() { return vec(0, .75, 1);}
	void hit(rpgent *attacker, use_weapon *weapon, use_weapon *ammo, float mul, int flags, vec dir);
	const int type() {return ENT_ITEM;}
	void init(int base);
	item *additem(int base, int q);
	item *additem(item *it);
	int getscript();
	int getcount(item *it);
	int getitemcount(int i);
	float getweight();

	rpgitem() : lastupdate(0) {physent::type = ENT_INANIMATE;}
	~rpgitem() {}
};

struct rpgobstacle : rpgent
{
	enum
	{
		F_STATIONARY = 1 << 0,
		F_MAX = (1 << 1) -1
	};

	const char *mdl;
	int weight;
	int script;
	int flags;
	int lastupdate;

	void update();
	void resetmdl() { temp.mdl = mdl;}
	void render(bool mainpass);
	int getscript() { return script;}
	vec blipcol() { return vec(1, 1, 1);}
	const char *getname() const { return NULL; }
	const int type() { return ENT_OBSTACLE; }
	void init(int base);

	///character/AI
	void givexp(int xp) {}
	void equip(int i, int u = 0) {}
	void die(rpgent *killer = NULL) {}
	void hit(rpgent *attacker, use_weapon *weapon, use_weapon *ammo, float mul, int flags, vec dir);


	rpgobstacle() : mdl(newstring(DEFAULTMODEL)), weight(100), script(3), flags(0), lastupdate(0) { physent::type = ENT_INANIMATE;}
	~rpgobstacle() { delete[] mdl; }
};

struct rpgcontainer : rpgent
{
	const char *mdl, *name;
	hashtable<int, vector<item *> > inventory;
	int capacity;
	int faction; //defines owner
	int merchant;
	int script, lock;
	int magelock; //lock via spells

	void update();
	void resetmdl();
	void render(bool mainpass);
	int getscript() { return script;}
	vec blipcol() { return vec(1, 1, 1);}
	const char *getname() const { return name; }
	const int type() { return ENT_CONTAINER; }
	void init(int base);
	//void getsignal(const char *sig, bool prop = true, rpgent *sender = NULL) {}

	///character/AI
	void givexp(int xp) {}
	void equip(int i, int u = 0) {}
	void die(rpgent *killer = NULL) {}
	void hit(rpgent *attacker, use_weapon *weapon, use_weapon *ammo, float mul, int flags, vec dir);

	///inventory
	item *additem(item *it);
	item *additem(int base, int q);
	int drop(item *it, int q, bool spawn);
	int drop(int base, int q, bool spawn);
	int getitemcount(int base);
	int getcount(item *it);
	float getweight();

	rpgcontainer() : mdl(newstring(DEFAULTMODEL)), name(NULL), capacity(200), faction(-1), merchant(-1), script(4), lock(0), magelock(0) {physent::type = ENT_INANIMATE;}
	~rpgcontainer()
	{
		delete[] name;
		delete[] mdl;
		enumerate(inventory, vector<item *>, stack, stack.deletecontents());
	}
};

struct rpgplatform : rpgent
{
	enum
	{
		F_ACTIVE = 1 << 0,
		F_MAX = (1 << 1) -1
	};

	const char *mdl;
	int speed;
	int script, flags;

	hashtable<int, vector<int> > routes;

	vector<rpgent *> stack;
	int target;

	void update();
	void resetmdl() { temp.mdl = (mdl && mdl[0]) ? mdl : DEFAULTMODEL; }
	void render(bool mainpass);
	int getscript() { return script;}
	vec blipcol() { return vec(1, 1, 1);}
	const char *getname() const { return NULL; }
	const int type() { return ENT_PLATFORM; }
	void init(int base);

	///character/AI
	void givexp(int xp) {}
	void equip(int i, int u = 0) {}
	void die(rpgent *killer = NULL) {}
	void hit(rpgent *attacker, use_weapon *weapon, use_weapon *ammo, float mul, int flags, vec dir);

	rpgplatform() : mdl(newstring(DEFAULTMODEL)), speed(100), script(5), flags(0), routes(hashtable<int, vector<int> >(16)), target(-1) {physent::type = ENT_INANIMATE;}
	~rpgplatform() { delete[] mdl; }
};

struct rpgtrigger : rpgent
{
	enum
	{
		F_INVIS = 1 << 0,
		F_TRIGGERED = 1 << 1,
		F_MAX = (1 << 2) -1
	};

	const char *mdl, *name;
	int script, flags;
	int lasttrigger;

	void update();
	void resetmdl() { temp.mdl = (mdl && mdl[0]) ? mdl : DEFAULTMODEL; }
	void render(bool mainpass);
	int getscript() { return script;}
	vec blipcol() { return vec(1, 1, 1);}
	const char *getname() const { return name; }
	const int type() { return ENT_TRIGGER; }
	void init(int base);

	///character/AI
	void givexp(int xp) {}
	void equip(int i, int u = 0) {}
	void die(rpgent *killer = NULL) {}
	void hit(rpgent *attacker, use_weapon *weapon, use_weapon *ammo, float mul, int flags, vec dir);

	rpgtrigger() : mdl(newstring(DEFAULTMODEL)), name(NULL), script(6), flags(0), lasttrigger(lastmillis) {physent::type = ENT_INANIMATE;}
	~rpgtrigger()
	{
		delete[] mdl;
		delete[] name;
	}
};

struct ammotype
{
	const char *name;
	vector<int> items;

	ammotype() : name(NULL) {}
	~ammotype() { delete[] name; }
};

struct faction
{
	const char *name, *logo;
	vector<short> relations;
	int base;

	void setrelation(int i, int f)
	{
		if(i < 0)
			return;
		if(i >= relations.length())
		{
			int olen = relations.length();
			relations.growbuf(i + 1);
			relations.advance(relations.capacity() - olen); // this would max it out to capacity

			loopi(relations.length()-olen)
			relations[i + olen] = base; //default to neutral
		}

		relations[i] = min(100, max(0, f));
	}

	int getrelation(int i)
	{
		if(relations.inrange(i))
			return relations[i];

		return base; //neutral
	}

	faction() : name(NULL), logo(NULL), base(50) {}
	~faction()
	{
		delete[] name;
		delete[] logo;
	}
};

//directives are sorted by priority, the most important ones are done first.
//for example, self defense is high priority while fleeing is an even higher priority,
//directives also include rudimentary things such as picking stuff up
struct directive
{
	int priority;

	directive(int p) : priority(p) {}
	virtual ~directive() {}

	virtual int type()=0;
	virtual bool match(directive *)=0;
	virtual bool update(rpgchar *d)=0; // returns false when executed/finished, some (such as follow) don't finish unless explicitly cleared

	static bool compare(directive *a, directive *b)
	{
		return a->priority > b->priority;
	}
};

enum
{
	AI_MOVE = 1,
	AI_ALERT = 2,
	AI_ATTACK = 4
}; //AI flags

struct rpgchar : rpgent
{
	vector<equipment*> equipped;
	vec emitters[SLOT_NUM * 2]; //two emitters a slot; start and end

	const char *name;
	const char *mdl;
	const char *portrait;
	stats base;
	hashtable<int, vector<item *> > inventory;
	int script, faction, merchant;

	float health, mana;

	int lastaction, lastai;
	int charge;
	bool primary, secondary;
	bool lastprimary, lastsecondary;

	use_weapon *attack;

	//AI specific stuff, the player can access them during (and only during) cutscenes

	int aiflags;
	rpgent *target;
	vec dest, lastknown;
	vector<ushort> route; //reversed
	vector<directive *> directives;

	//AI functions

	bool cansee(rpgent *d);
	void doai(equipment *eleft, equipment *eright, equipment *quiver);

	///global
	void resetmdl();
	void update();
	void render(bool mainpass);
	const char *getname() const;
	const int type() {return ENT_CHAR;}
	int getscript() {return script;}
	void init(int base);

	///character/AI
	void givexp(int xp);
	void equip(item *it, int u = 0);
	bool dequip(int base, int slots = 0);
	void die(rpgent *killer);
	void revive(bool spawn = true);
	void hit(rpgent *attacker, use_weapon *weapon, use_weapon *ammo, float mul, int flags, vec dir);

	///inventory
	item *additem(item *it);
	item *additem(int base, int q);
	int drop(item *it, int q, bool spawn);
	int drop(int base, int q, bool spawn);
	int pickup(rpgitem *it);
	int getitemcount(int base);
	int getcount(item *it);
	float getweight();

	//character specific stuff
	bool checkammo(equipment &eq, equipment *quiver, bool remove = false);
	void doattack(equipment *eleft, equipment *eright, equipment *quiver);

	void cleanup();

	rpgchar() : name(NULL), mdl(newstring(DEFAULTMODEL)), portrait(NULL), base(stats(this)), script(1), faction(0), merchant(-1), lastaction(0), lastai(0), charge(0), primary(false), secondary(false), lastprimary(false), lastsecondary(false), attack(NULL), aiflags(0), target(NULL)
	{
		health = base.getmaxhp();
		mana = base.getmaxmp();
		loopi(SLOT_NUM * 2) emitters[i] = vec(-1, -1, -1);
	}
	~rpgchar()
	{
		delete[] name;
		delete[] mdl;
		delete[] portrait;
		enumerate(inventory, vector<item *>, stack, stack.deletecontents());
		directives.deletecontents();
		equipped.deletecontents();
	}
};

struct merchant
{
	struct rate
	{
		float buy, sell;
		rate(float b, float s) : buy(b), sell(s) {}
	};

	vector<rate> rates;
	int currency;
	int credit; //store credit, a measure of how much money the merchant owes you.

	rate getrate(rpgchar &buyer, int cat)
	{
		if(!rates.inrange(cat))
			return rate(0, 0);

		rate r = rates[cat];
		float diff = (r.buy - r.sell) * clamp(buyer.base.getskill(SKILL_DIPLOMACY), 0, 100) / 200.0f;

		r.buy -= diff;
		r.sell += diff;
		return r;
	}

	merchant() : currency(0), credit(0) {}
};

struct recipe
{
	enum
	{
		KNOWN = 1 << 0,
		SINGLE = 1 << 1, //recipe forgotten after use
		MAX = (1 << 2) -1,

		SAVE = KNOWN //flags which are to be saved
	};

	struct ingredient
	{
		int base, quantity;
		ingredient(int b, int q) : base(b), quantity(q) {}
	};

	vector<ingredient> ingredients, // meat.. veggies.. seasoning... butane...
		catalysts, // portable gas stove
		products; // stew! YEEAAH

	const char *name;
	statreq reqs;
	int flags;

	recipe() : name(NULL), flags(0) {}
	~recipe() { delete[] name; }

	static void optimise(vector<ingredient> &list)
	{
		loopv(list)
		{
			for(int j = i + 1; j < list.length(); j++)
			{
				if(list[i].base == list[j].base)
				{
					list[i].quantity += list[j].quantity;
					list.remove(j);
					j--;
				}
			}
		}
	}

	inline void optimise()
	{
		optimise(ingredients);
		optimise(catalysts);
		optimise(products);
	}

	bool checkreqs(rpgchar *d, int mul)
	{
		if(!(flags & KNOWN)) return false;
		if(!reqs.meet(d->base)) return false;

		vector<ingredient> list;
		loopv(ingredients) list.add(ingredient(ingredients[i])).quantity *= mul;
		loopv(catalysts) list.add(ingredient(catalysts[i]));
		optimise(list);

		loopv(list)
		{
			if(d->getitemcount(list[i].base) < list[i].quantity)
				return false;
		}

		return true;
	}

	void craft(rpgchar *d, int mul)
	{
		if(!checkreqs(d, mul)) return;
		if(flags & SINGLE) flags &= ~KNOWN;

		loopv(ingredients)
			d->drop(ingredients[i].base, ingredients[i].quantity * mul, false);

		loopv(products)
			d->additem(products[i].base, products[i].quantity * mul);
	}
};

enum
{
	ACTION_TELEPORT = 0,
	ACTION_SPAWN,
	ACTION_SCRIPT
};

struct action
{
	virtual void exec()=0;
	virtual const int type()=0;

	action() {}
	virtual ~action() {}
};

//actions to execute when the player enters a map
struct action_teleport : action
{
	rpgent *ent;
	int dest;

	void exec();
	const int type() {return ACTION_TELEPORT;}

	action_teleport(rpgent *e, int d) : ent(e), dest(d) {}
	~action_teleport() {}
};

struct action_spawn : action
{
	// entity tag, entity type, base id
	int tag, ent, id, amount, qty;

	void exec();
	const int type() {return ACTION_SPAWN;}

	action_spawn(int ta, int en, int i, int amt, int q) : tag(ta), ent(en), id(i), amount(amt), qty(q) {}
	~action_spawn()	{}
};

struct action_script : action
{
	const char *script;

	void exec() { if(script) execute(script); }
	const int type() {return ACTION_SCRIPT;}

	action_script(const char *s) : script(newstring(s)) {}
	~action_script()
	{
		delete[] script;
	}
};

struct blip
{
	vec o;
	vec col;
	int size;
	//Texture *icon;

	blip(vec _o, vec _c = vec(1,1,1), int _s = 8) : o(_o), col(_c), size(_s) {}
	~blip() {}
};

struct mapinfo
{
	enum
	{
		F_VOLATILE = 1 << 0, //nothing persists - everything is cleared when left
		F_NOSAVE = 1 << 1, //perfect for dungeon levels
		F_NOMINIMAP = 1 << 2,
		F_MAX = (1 << 3) - 1
	};

	// we track the map's key here; we risk leaking memory otherwise
	const char *name;
	vector<rpgent *> objs;
	int script, flags;
	bool loaded;
	vector<action *> loadactions;
	vector<projectile *> projs;
	vector<areaeffect *> aeffects;
	vector<blip> blips;

	void getsignal(const char *sig, bool prop = true, rpgent *sender = NULL);
	mapinfo() : name(NULL), script(0), flags(0), loaded(false) {}
	~mapinfo()
	{
		delete[] name;
		objs.deletecontents();
		loadactions.deletecontents();
		projs.deletecontents();
		aeffects.deletecontents();
	}
};

static inline bool htcmp(const char *key, const mapinfo &ref) { return !strcmp(key, ref.name); }

//this is compatible with the FPSGAME waypoints
struct waypoint
{
	waypoint *parent;
	vec o;
	vector<ushort> links;
	int score;

	waypoint() {}
	waypoint(const vec &o) : parent(NULL), o(o), score(-1) {}
};

struct reference
{
	enum //identifies type and valid casts
	{
		T_INVALID = 0,
		T_CHAR,
		T_ITEM,
		T_OBSTACLE,
		T_CONTAINER,
		T_PLATFORM,
		T_TRIGGER,
		T_INV,
		T_EQUIP,
		T_MAP,
		T_VEFFECT,
		T_AEFFECT
	};

	struct ref
	{
		void *ptr;
		int type;

		ref(void *p, int t) : ptr(p), type(t) {}
		ref() : ptr(NULL), type(T_INVALID) {}
	};

	const char *name;
	vector<ref> list;

	rpgchar *getchar(int i) const;
	rpgitem *getitem(int i) const;
	rpgobstacle *getobstacle(int i) const;
	rpgcontainer *getcontainer(int i) const;
	rpgplatform *getplatform(int i) const;
	rpgtrigger *gettrigger(int i) const;
	rpgent *getent(int i) const;
	item *getinv(int i) const;
	equipment *getequip(int i) const;
	mapinfo *getmap(int i) const;
	victimeffect *getveffect(int i) const;
	areaeffect *getaeffect(int i) const;

	void pushref(rpgchar *d);
	void pushref(rpgitem *d);
	void pushref(rpgobstacle *d);
	void pushref(rpgcontainer *d);
	void pushref(rpgplatform *d);
	void pushref(rpgtrigger *d);
	void pushref(rpgent *d);
	void pushref(item *d);
	void pushref(vector<item *> *d);
	void pushref(equipment *d);
	void pushref(mapinfo *d);
	void pushref(victimeffect *d);
	void pushref(areaeffect *d);
	void pushref(reference *d);
	inline void pushref(int) {setnull();} //for NULL

	template<typename T>
	inline void setref(T d) {list.setsize(0); pushref(d);}

	//specialised inside rpgscript.cpp
	void setnull();
	template<typename T>
	reference(const char *n, T d);
	reference(const char *n);
	reference() : name(NULL) {}

	~reference();
};

static inline bool htcmp(const char *key, const reference &ref) { return !strcmp(key, ref.name); }

struct delayscript
{
	hashset<reference> refs;
	const char *script;
	int remaining;

	bool update();
	delayscript() : refs(hashset<reference>(128)), script(NULL), remaining(0) {}
	~delayscript() { delete[] script; }
};

enum
{
	ANIM_BLOCK = ANIM_GAMESPECIFIC,
	ANIM_MHOLD, ANIM_MSTRIKE, ANIM_MCHARGE, ANIM_MCOOL, //swords and stuff
	ANIM_RHOLD, ANIM_RSTRIKE, ANIM_RCHARGE, ANIM_RCOOL, //bows and stuff
	ANIM_CHOLD, ANIM_CSTRIKE, ANIM_CCHARGE, ANIM_CCOOL, //spell casting

	ANIM_HIDLE, ANIM_HSTRIKE, ANIM_HCHARGE, ANIM_HCOOL, //HUD stuff
	NUMANIMS
};

static const char * const animnames[] =
{
	ANIMNAMES,
	"block",
	"melee hold", "melee strike", "melee charge", "melee cooldown",
	"range hold", "range strike", "range charge", "range cooldown",
	"cast hold", "cast strike", "cast charge", "cast cooldown",
	"hud idle", "hud strike", "hud charge", "hud cooldown"
};

#ifdef NO_DEBUG

#define DEBUG_WORLD   (false)
#define DEBUG_ENT     (false)
#define DEBUG_CONF    (false)
#define DEBUG_PROJ    (false)
#define DEBUG_SCRIPT  (false)
#define DEBUG_AI      (false)
#define DEBUG_IO      (false)
#define DEBUG_CAMERA  (false)
#define DEBUG_STATUS  (false)
#define DEBUG_VERBOSE (false)

#else

#define DEBUG_WORLD   (game::debug & (1 << 0))
#define DEBUG_ENT     (game::debug & (1 << 1))
#define DEBUG_CONF    (game::debug & (1 << 2))
#define DEBUG_PROJ    (game::debug & (1 << 3))
#define DEBUG_SCRIPT  (game::debug & (1 << 4))
#define DEBUG_AI      (game::debug & (1 << 5))
#define DEBUG_IO      (game::debug & (1 << 6))
#define DEBUG_CAMERA  (game::debug & (1 << 7))
#define DEBUG_STATUS  (game::debug & (1 << 8))
#define DEBUG_VERBOSE (game::debug & (1 << 9) || game::forceverbose)

#endif


#define DEBUG_VWORLD   (DEBUG_VERBOSE && DEBUG_WORLD)
#define DEBUG_VENT     (DEBUG_VERBOSE && DEBUG_ENT)
#define DEBUG_VCONF    (DEBUG_VERBOSE && DEBUG_CONF)
#define DEBUG_VPROJ    (DEBUG_VERBOSE && DEBUG_PROJ)
#define DEBUG_VSCRIPT  (DEBUG_VERBOSE && DEBUG_SCRIPT)
#define DEBUG_VAI      (DEBUG_VERBOSE && DEBUG_AI)
#define DEBUG_VIO      (DEBUG_VERBOSE && DEBUG_IO)
#define DEBUG_VCAMERA  (DEBUG_VERBOSE && DEBUG_CAMERA)
#define DEBUG_VSTATUS  (DEBUG_VERBOSE && DEBUG_STATUS)

#define DEBUG_MAX ((1 << 10) - 1)

namespace ai
{
	extern int dropwaypoints;

	extern vector<waypoint> waypoints;
	extern void loadwaypoints(const char *name, bool msg = false);
	extern void savewaypoints(const char *name);
	extern void findroute(int from, int to, vector<ushort> &route);
	extern waypoint *closestwaypoint(const vec &o);
	extern void renderwaypoints();
	extern void trydrop();
	extern void clearwaypoints();
}

namespace game
{
	extern string data;

	extern vector<script *> scripts; ///scripts, includes dialogue
	extern vector<effect *> effects; ///pretty particle effects for spells and stuff
	extern vector<statusgroup *> statuses; ///status effect definitions to transfer onto victims
	extern vector<ammotype *> ammotypes;
	extern vector<faction *> factions;
	extern vector<mapscript *> mapscripts;
	extern vector<recipe *> recipes;
	extern vector<merchant *> merchants;

	extern hashset<journal> journals;
	extern vector<const char *> variables, categories, tips;

	extern vector<equipment> hotkeys;

	//config
	extern script *loadingscript;
	extern effect *loadingeffect;
	extern statusgroup *loadingstatusgroup;
	extern item *loadingitem;
	extern ammotype *loadingammotype;
	extern rpgchar *loadingrpgchar;
	extern faction *loadingfaction;
	extern rpgobstacle *loadingrpgobstacle;
	extern rpgcontainer *loadingrpgcontainer;
	extern rpgplatform *loadingrpgplatform;
	extern rpgtrigger *loadingrpgtrigger;
	extern mapscript *loadingmapscript;
	extern recipe *loadingrecipe;
	extern merchant *loadingmerchant;

	extern int debug, forceverbose;
	extern bool connected;
	extern bool transfer;
	extern rpgchar *player1;
	extern hashset<mapinfo> *mapdata;
	extern mapinfo *curmap;

	extern void openworld(const char *name, bool fall = false);
	extern void newgame(const char *game, bool restore = false);
	extern mapinfo *accessmap(const char *name);
	extern bool cansave();
	extern bool intersect(rpgent *d, const vec &from, const vec &to, float &dist);
	extern rpgent *intersectclosest(const vec &from, const vec &to, rpgent *at, float &bestdist, float maxdist = 1e16);
	extern void loadcutscenes(const char *dir);
	extern void hudline(const char *fmt, ...);

	//game variables
	extern char *firstmap;
	extern int gameversion;
	extern int compatversion;
}

namespace rpgscript
{
	extern reference *hover, *talker, *looter, *trader;
	extern vector<rpgent *> obits;
	extern void parseref(const char *name, int &idx);
	extern reference *searchstack(const char *name, bool create = false);

	extern void update();
	extern void clean();
	extern void init();
	extern void removereferences(rpgent *ptr, bool references = true);
	extern void removeminorrefs(void *ptr);
	extern void changemap();
	extern void doitemscript(item *invokee, rpgent *invoker, uint *code);
	extern void doentscript(rpgent *invokee, rpgent *invoker, uint *code);
	extern void domapscript(mapinfo *invokee, rpgent *invoker, uint *code);

	extern void pushstack();
	extern void popstack();
	extern vector<hashset<reference> *> stack;
	extern vector<delayscript *> delaystack;
}

namespace entities
{
	extern vector<extentity *> ents;
	extern vector<int> intents;
	extern void startmap();
	extern void spawn(extentity &e, int ind, int type, int qty);
	extern void teleport(rpgent *d, int dest);
	extern void genentlist();
	extern void touchents(rpgent *d);
	extern void renderentities();
}

namespace camera
{
	extern bool cutscene;
	extern void cleanup(bool clear);
	extern void update();
	extern void render(int w, int h);
	extern void getsignal(const char *signal);

	extern physent *attached;
	extern physent camera;
}

namespace rpggui
{
	bool open();
	void forcegui();
	bool hotkey(int n);
}

enum
{
	S_JUMP = 0, S_LAND, S_RIFLE, S_TELEPORT, S_SPLASH1, S_SPLASH2, S_CG,
	S_RLFIRE, S_RUMBLE, S_JUMPPAD, S_WEAPLOAD, S_ITEMAMMO, S_ITEMHEALTH,
	S_ITEMARMOUR, S_ITEMPUP, S_ITEMSPAWN,  S_NOAMMO, S_PUPOUT,
	S_PAIN,
	S_DIE,
	S_FLAUNCH, S_FEXPLODE,
	S_SG, S_PUNCH1,
	S_GRUNT1, S_GRUNT2, S_RLHIT,
	S_PAINO,
	S_PAINR, S_DEATHR,
	S_PAINE, S_DEATHE,
	S_PAINS, S_DEATHS,
	S_PAINB, S_DEATHB,
	S_PAINP, S_PIGGR2,
	S_PAINH, S_DEATHH,
	S_PAIND, S_DEATHD,
	S_PIGR1, S_ICEBALL, S_SLIMEBALL, S_PISTOL,

	S_V_BASECAP, S_V_BASELOST,
	S_V_FIGHT,
	S_V_BOOST, S_V_BOOST10,
	S_V_QUAD, S_V_QUAD10,
	S_V_RESPAWNPOINT,

	S_FLAGPICKUP,
	S_FLAGDROP,
	S_FLAGRETURN,
	S_FLAGSCORE,
	S_FLAGRESET,

	S_BURN,
	S_CHAINSAW_ATTACK,
	S_CHAINSAW_IDLE,

	S_HIT
};
#endif