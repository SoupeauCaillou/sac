#pragma once

#include "stdafx.h"
#include "DebugRenderingManager.h"

#define SINGLETON(T) static T& GetInstance() { if (_instance == NULL) _instance = new T(); return (*_instance); } 
#define INSTANCE_DECL(T) static T* _instance;
#define INSTANCE_IMPL(T) T* T::_instance = 0;

#define SYSTEM(type) \
	class type##System : public ComponentSystem<type##Component> {	\
		public:	\
			static type##System& GetInstance() { if (_instance == NULL) _instance = new type##System(); return (*_instance); } \
		private:	\
			type##System();	\
			static type##System* _instance;
			
#define UPDATABLE_SYSTEM(type) \
	class type##System : public ComponentSystem<type##Component> {	\
		public:	\
			static type##System& GetInstance() { if (_instance == NULL) _instance = new type##System(); return (*_instance); } \
			void Update(float dt) {  if(active) DoUpdate(dt); }	\
		\
		protected:\
			void DoUpdate(float dt); \
		private:	\
			type##System();	\
			static type##System* _instance;

#define UPDATABLE_RENDERABLE_SYSTEM(type) \
	class type##System : public ComponentSystem<type##Component>, public Renderable {	\
		public:	\
			static type##System& GetInstance() { if (_instance == NULL) {\
				_instance = new type##System(); \
				DebugRenderingManager::Instance().RegisterDebugRenderer(#type"SystemDebugRenderMsg", _instance);\
			} \
			return (*_instance); \
			}\
			void Update(float dt) {  if(active) DoUpdate(dt); }	\
			void Render(); \
		\
		protected:\
			void DoUpdate(float dt); \
		private:	\
			type##System();	\
			static type##System* _instance;
			
template <typename T> 
class ComponentSystem : public MessageListener {
	public:
		ComponentSystem(const std::string& t) : MessageListener(), tag(t) { 
			Activate();
		}
		
		virtual void ReceiveMessage(Message* m) { }
				
		void Add(Actor* actor) {
			components[actor] = new T();
			actor->Tag(tag);
		}
		
		void Delete(Actor* actor) {
			if (actor->IsTagged(tag)) {
				actor->Untag(tag);
				T* t = Get(actor);
				components.erase(actor);
				delete t;
			}
		}
		
		T* Get(Actor* actor) {
			typename std::map<Actor*, T*>::iterator it = components.find(actor);
			if (it == components.end()) {
				// std::cout << "Actor " << actor << " has no component of type " << tag << std::endl;
				return 0;
			}
			return (*it).second;
		}
		
		std::vector<Actor*> RetrieveAllActorWithComponent() {
			std::vector<Actor*> result;
			for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
				result.push_back((*it).first);
			}
			return result;
		}
		
		void Clear() { components.clear(); }
	
		void Activate() { active = true; activationTime = theWorld.GetCurrentTimeSeconds(); }
		void Deactivate() { active = false; }
		bool IsActive() const { return active; }
		const String& GetTag() const { return tag; }
		
	protected:
		bool active;
	
		std::string tag;
		std::map<Actor*, T*> components;

		typedef typename std::map<Actor*, T*> ComponentMap;
		typedef typename std::map<Actor*, T*>::iterator ComponentIt;
		typedef typename std::map<Actor*, T*>::const_iterator ComponentConstIt;
		
		float activationTime;
};

